/***************************************************************
 * MOS 6502 System Emulator
 * v 0.1
 *
 * (c) 2017 Andreas J. Reichel
 *
 * h o m e b a s e _ a r (a|t]> w e b . d e
 *
 * License: MIT (see LICENSE.txt) 
 ***************************************************************/

#include <pthread.h>
#include <stdio.h>
#include <math.h>
#include <argp.h>

#include <gtk/gtk.h>

#include "6502.h"

#define F_CYC_NSEC 1000

uint8_t memory[65536];

void dumpRegs()
{
    printf("S = %02X  A = %02X   X = %02X   Y = %02X  ", cpu.S, cpu.A, cpu.X, cpu.Y);
    printf("FLAGS = %c%c%c%c%c%c%c   ", 
        cpu.P & F_N ? 'N' : ' ',
        cpu.P & F_V ? 'V' : ' ',
        cpu.P & F_B ? 'B' : ' ',
        cpu.P & F_D ? 'D' : ' ',
        cpu.P & F_I ? 'I' : ' ',
        cpu.P & F_Z ? 'Z' : ' ',
        cpu.P & F_C ? 'C' : ' ');
}


const char *argp_program_version = "sim6502c 0.01 - Alpha";
const char doc[] = "sim6502c - an extendable 6502 simulator in C";

static struct argp_option options[] = {
    {"rom", 'r', "FILE", 0, "Specify ROM file"},
    {"raddr", 'a', "HEX", 0, "Specify ROM address in HEX, default is C000"},
    {0}
};

struct arguments
{
    char *romfile;
    uint16_t romaddr;
    bool debug_regs;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    struct arguments *arguments = state->input;

    switch(key)
    {
    case 'r':
        arguments->romfile = arg;
        fprintf(stdout, "ROM is loaded from %s\n", arguments->romfile);
        break;
    case 'a':
        arguments->romaddr = strtol(arg, NULL, 16);
        fprintf(stdout, "ROM starts at %u\n", arguments->romaddr);
        break;
    case 'd':
        arguments->debug_regs = true;
        break;
    case ARGP_KEY_ARG:
        if (state->arg_num > 0) {
            argp_usage(state);
        }
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = {
    options, parse_opt, 0, doc
};

volatile bool sim_running = true;
volatile bool wait_clock = false;
volatile long clock_loop, clock_loop_times;

void *timer_func(void *threadid)
{
    struct timespec wait_time;
            
    while(sim_running) {

        for (long i = 0; i < clock_loop_times; i++) {
            asm("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop");
        }

        wait_clock = false;
        while(!wait_clock & sim_running);
    }
}


static cairo_surface_t *surface = NULL;


static void close_window(void)
{
    if (surface) {
        cairo_surface_destroy(surface);
    }
}

static void clear_surface()
{
    cairo_t *cr;

    cr = cairo_create(surface);

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    cairo_destroy(cr);
}

static gboolean draw_cb(GtkWidget *widget, cairo_t *cr, gpointer data)
{
/*    cairo_set_source_surface(cr, surface, 0, 0);
    cairo_paint(cr);
*/
    cairo_set_source_rgb(cr, 0, 0, 1);
    cairo_set_line_width(cr, 8);
    cairo_rectangle(cr, 0, 0, 210, 156);
    cairo_stroke_preserve(cr);
    cairo_fill(cr);

    return FALSE;
}

static gboolean configure_cb(GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
    if (surface) {
        cairo_surface_destroy(surface);
    }

    surface = gdk_window_create_similar_surface(gtk_widget_get_window(widget),
                                                CAIRO_CONTENT_COLOR,
                                                gtk_widget_get_allocated_width(widget),
                                                gtk_widget_get_allocated_height(widget));

    clear_surface();

    return TRUE;
}

void initMainWindow()
{
    GtkWidget *main_window;

    gtk_init(0, NULL);

    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), "RGA Emulation Window");
    gtk_window_set_default_size(GTK_WINDOW(main_window), 420, 312);

    g_signal_connect(main_window, "destroy", G_CALLBACK(close_window), NULL);
    gtk_container_set_border_width(GTK_CONTAINER(main_window), 8);

    GtkWidget *frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
    gtk_container_add(GTK_CONTAINER(main_window), frame);


    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 420, 312);

    gtk_container_add(GTK_CONTAINER(frame), drawing_area);

    g_signal_connect(drawing_area, "draw", G_CALLBACK(draw_cb), NULL);
    //g_signal_connect(drawing_area, "configure-event", G_CALLBACK(configure_cb), NULL);

    gtk_widget_show_all(main_window);
}

void *gtk_main_func(void *data)
{
    while(sim_running) {
        if (gtk_events_pending()) {
            gtk_main_iteration();
        }
    }

    pthread_exit(NULL);
}

int main(int argc, char **argv)
{
    pthread_t timer_thread;
    pthread_t gtk_thread;

    struct arguments arguments;

    arguments.romfile = NULL;
    arguments.romaddr = 0xC000;
    arguments.debug_regs = false;

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    bool done;
    uint8_t cc, idx;
    enum amode a;
    uint8_t op, cycles;
    struct timespec time;

    initMainWindow();

    /* Get timer resolution */
    clock_getres(CLOCK_REALTIME, &time);
    fprintf(stdout, "Realtime clock resolution: %lu ns\n", time.tv_nsec);

    /* Callibrate time delay */
    struct timespec oldtime;
    clock_gettime(CLOCK_REALTIME, &oldtime);
    for (long j = 0; j < 5E8; j++) {
        asm("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop");
    }
    clock_gettime(CLOCK_REALTIME, &time);

    long delta_ns = time.tv_nsec - oldtime.tv_nsec;
    long delta_s = time.tv_sec - oldtime.tv_sec;
    double delta = (double) delta_s + 1.0E-9 * delta_ns;
    printf("delta = %f\n", delta);

    double factor = F_CYC_NSEC * 1E-9 / delta;
    printf("factor = %.9f\n", factor);

    clock_loop = (long) round(5.0E8 * factor);
    printf("clock_loop = %lu\n", clock_loop);
    clock_loop_times = clock_loop * 10;

    /* Initialize stack pointer */
    cpu.S = 0xFF;

    /* Initialize processor status (bit 5 is always 1) */
    cpu.P = 0x20;

    /* Read ROM from file */
    FILE *romfile = fopen(arguments.romfile, "r");
    if (!romfile) {
        fprintf(stderr, "Could not load ROM from file.\n");
        exit(1);
    }
    size_t res = fread(&memory[arguments.romaddr], 1, 0xFFFF - arguments.romaddr + 1, romfile);
    fclose(romfile);
    fprintf(stdout, "ROM: %u bytes read.\n", res);

    /* Check if ROM has correct size and if vectors are present */
    if (res != 0xFFFF - arguments.romaddr + 1) {
        fprintf(stderr, "Your ROM file is too small, no data at vector table.\n");
        exit(1);
    }

    /* Fetch reset Vector */
    uint16_t start_addr = *((uint16_t *) &memory[0xFFFC]);
    cpu.PC = start_addr;

    /* Create timer thread */
    int rc = pthread_create(&timer_thread, NULL, timer_func, NULL);
    if (rc) {
        fprintf(stderr, "Cannot create timer thread.\n");
        exit(-1);
    }

    /* Create gtk thread */
    rc = pthread_create(&gtk_thread, NULL, gtk_main_func, NULL);
    if (rc) {
        fprintf(stderr, "Cannot create gtk thread.\n");
        exit(-1);
    }

    long oldns;
    long cdelta;
    while(cpu.PC != 0xFFFF) {
        clock_gettime(CLOCK_REALTIME, &time);

        /* fine tuning */
        cdelta = time.tv_nsec - oldns;
        if (cdelta > 0) {
            if (cdelta > F_CYC_NSEC * cycles) {
                clock_loop -= clock_loop / 4;
            }
            if (cdelta < F_CYC_NSEC * cycles) {
                clock_loop += clock_loop / 4;
            }
        }
        oldns = time.tv_nsec;
        printf("\n%lu,%09lu - ", time.tv_sec, time.tv_nsec);

        // fetch the opcode from memory
        op = memory[cpu.PC];
        printf("%04X : %02X ", cpu.PC, op);

        // each opcode can be understood as aaabbbcc, where
        // bbb gives the addressing mode and ccaaa the instruction

        // extract cc
        cc = op & 3;

        // rol by 2 (opcode becomes ccaaabbb)
        op >>= 2;
        op |= cc << 6;

        // exctract instruction selector (ccaaa)
        idx = op >> 3;

        // idx specifies instruction, op & 7 is bbb, which gives
        // adressing mode
        a = opcodes[idx].addr_modes[op & 7];

        // bbb also gives number of cycles
        cycles = opcodes[idx].cycles[op & 7];
        printf(" CYC = %u ", cycles);

        // calculate needed timing loop counts
        clock_loop_times = clock_loop * cycles;

        if (arguments.debug_regs) {
            dumpRegs();
        }

        // signal the timing thread that it can start counting
        wait_clock = true;

        // parallely, execute CPU instruction
        opcodes[idx].func(a, op & 7);

        // wait remaining counts done by timing thread
        while(wait_clock);
    }

    // signal the timing thread that it must stop
    sim_running = false;

    pthread_exit(NULL);
}

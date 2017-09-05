#include "6502.h"
#include "sim.h"
#include "gui.h"

static cairo_surface_t *surface = NULL;
static GtkTextBuffer *text_buff;
static GtkTextBuffer *status_text_buff;
GtkWidget *main_window;

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
    

GtkWidget *initMainmenu()
{
    GtkWidget *file_menu;

    file_menu = gtk_menu_new();
    /* Create the menu items */
    GtkWidget *open_item = gtk_menu_item_new_with_label ("Open");
    GtkWidget *save_item = gtk_menu_item_new_with_label ("Save");
    GtkWidget *quit_item = gtk_menu_item_new_with_label ("Quit");

    /* Add them to the menu */
    gtk_menu_shell_append (GTK_MENU_SHELL(file_menu), open_item);
    gtk_menu_shell_append (GTK_MENU_SHELL(file_menu), save_item);
    gtk_menu_shell_append (GTK_MENU_SHELL(file_menu), quit_item);

    GtkWidget *menu_bar = gtk_menu_bar_new();

    GtkWidget *file_item = gtk_menu_item_new_with_label ("File");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file_item);

    return menu_bar;
}

void initMainWindow(void)
{

    gtk_init(0, NULL);

    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), "65simC");
    gtk_window_set_default_size(GTK_WINDOW(main_window), 1024, 768);

    GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(main_window), vbox);
 

    g_signal_connect(main_window, "destroy", G_CALLBACK(close_window), NULL);
    gtk_container_set_border_width(GTK_CONTAINER(main_window), 8);

    GtkWidget *main_menu = initMainmenu();
//    gtk_container_add(GTK_CONTAINER(main_window), main_menu);
    gtk_box_pack_start(GTK_BOX(vbox), main_menu, FALSE, FALSE, 0);


    GtkWidget *frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);

    GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(vbox), hbox);


//    gtk_container_add(GTK_CONTAINER(hbox), frame);


//    GtkWidget *drawing_area = gtk_drawing_area_new();
//    gtk_widget_set_size_request(drawing_area, 420, 312);

//    gtk_container_add(GTK_CONTAINER(frame), drawing_area);

//    g_signal_connect(drawing_area, "draw", G_CALLBACK(draw_cb), NULL);
    //g_signal_connect(drawing_area, "configure-event", G_CALLBACK(configure_cb), NULL);

    text_buff = gtk_text_buffer_new(NULL);
    status_text_buff = gtk_text_buffer_new(NULL);
   
    GtkWidget *text_widget = gtk_text_view_new_with_buffer(text_buff);
    GtkWidget *status_widget = gtk_text_view_new_with_buffer(status_text_buff);

    gtk_text_view_set_editable((GtkTextView *)text_widget, FALSE);
    gtk_text_view_set_editable((GtkTextView *)status_widget, FALSE);

    gtk_text_view_set_cursor_visible((GtkTextView *)text_widget, FALSE);
    gtk_text_view_set_cursor_visible((GtkTextView *)status_widget, FALSE);
    
    GdkRGBA gcolor = {0.0, 0.05, 0.0, 1.0};
    GdkRGBA gcolor2 = {0.3, 1.0, 0.3, 1.0};
    gtk_widget_override_background_color(text_widget, GTK_STATE_FLAG_NORMAL, &gcolor);
    gtk_widget_override_color(text_widget, GTK_STATE_FLAG_NORMAL, &gcolor2);
    
    GdkRGBA gcolor3 = {0.0, 0.0, 0.05, 1.0};
    GdkRGBA gcolor4 = {0.3, 0.3, 1.0, 1.0};
    gtk_widget_override_background_color(status_widget, GTK_STATE_FLAG_NORMAL, &gcolor3);
    gtk_widget_override_color(status_widget, GTK_STATE_FLAG_NORMAL, &gcolor4);

    gtk_container_add(GTK_CONTAINER(hbox), status_widget);
    gtk_container_add(GTK_CONTAINER(hbox), text_widget);

    PangoFontDescription *df;

    df = pango_font_description_from_string("Monospace");

    pango_font_description_set_size(df, 12*PANGO_SCALE);
    gtk_widget_override_font(text_widget, df);
    gtk_widget_override_font(status_widget, df);

    gtk_widget_show_all(main_window);
}

void *gtk_main_func(void *data)
{
    char buffer[256];
    GtkTextIter start_iter;
    GtkTextIter end_iter;
    
    while(sim_running) {

        gtk_text_buffer_get_start_iter(text_buff, &start_iter);
        gtk_text_buffer_get_end_iter(text_buff, &end_iter);
        gtk_text_buffer_delete(text_buff, &start_iter, &end_iter);
        for (uint16_t i = 0; i < 0x100; i++) {
                if (i % 0x10 == 0) {
                    sprintf(buffer, "\n %02X : ", i);
                    gtk_text_buffer_insert(text_buff, &start_iter, buffer, -1);
                }
                sprintf(buffer, "%02X ", memory[i]);
                gtk_text_buffer_insert(text_buff, &start_iter, buffer, -1);
        }
        gtk_text_buffer_get_start_iter(status_text_buff, &start_iter);
        gtk_text_buffer_get_end_iter(status_text_buff, &end_iter);
        gtk_text_buffer_delete(status_text_buff, &start_iter, &end_iter);
        sprintf(buffer, "\n PC = %04X\n S = %02X  A = %02X\n X = %02X   Y = %02X  ",
                cpu.PC, cpu.S, cpu.A, cpu.X, cpu.Y);
        gtk_text_buffer_insert(status_text_buff, &start_iter, buffer, -1);
        sprintf(buffer, "\n FLAGS = %c%c%c%c%c%c%c   ", 
        cpu.P & F_N ? 'N' : ' ',
        cpu.P & F_V ? 'V' : ' ',
        cpu.P & F_B ? 'B' : ' ',
        cpu.P & F_D ? 'D' : ' ',
        cpu.P & F_I ? 'I' : ' ',
        cpu.P & F_Z ? 'Z' : ' ',
        cpu.P & F_C ? 'C' : ' ');
        gtk_text_buffer_insert(status_text_buff, &start_iter, buffer, -1);

//        gtk_widget_queue_draw(main_window);
    
        for (int i = 0; i < 1000; i++) {
            gtk_main_iteration_do(FALSE);
        }
    }

    pthread_exit(NULL);
}


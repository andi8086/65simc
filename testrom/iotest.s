    .SETCPU "6502"
    .SEGMENT "CODE"


TEST:             
        LDA #'A'
        STA $8020

        JMP $FFFF



__nmi:
    NOP
    RTI

__irq:
    NOP
    RTI

    .SEGMENT "VECTORS"
    .word __nmi
    .word TEST
    .word __irq

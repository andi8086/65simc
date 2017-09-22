    .SETCPU "6502"
    .SEGMENT "CODE"


TEST:             
        LDA #'A'
        STA $8020
        
        LDA #'B'
        STA $8020

READBYTE:
        LDA $8020
        NOP
        JMP READBYTE

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

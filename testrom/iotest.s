    .SETCPU "6502"
    .SEGMENT "CODE"


TEST:   CLD        ; Clear decimal mode for test
        LDA #13
        STA $8020

        LDA #14
        ASL $8021

        JMP $FFFF;



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

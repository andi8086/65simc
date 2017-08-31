; Verify decimal mode behavior

; Tests for ADC

;    00 + 00 and C=0 gives 00 and N=0 V=0 Z=1 C=0
;    79 + 00 and C=1 gives 80 and N=1 V=1 Z=0 C=0
;    24 + 56 and C=0 gives 80 and N=1 V=1 Z=0 C=0
;    93 + 82 and C=0 gives 75 and N=0 V=1 Z=0 C=1
;    89 + 76 and C=0 gives 65 and N=0 V=0 Z=0 C=1
;    89 + 76 and C=1 gives 66 and N=0 V=0 Z=1 C=1
;    80 + f0 and C=0 gives d0 and N=0 V=1 Z=0 C=1
;    80 + fa and C=0 gives e0 and N=1 V=0 Z=0 C=1
;    2f + 4f and C=0 gives 74 and N=0 V=0 Z=0 C=0
;    6f + 00 and C=1 gives 76 and N=0 V=0 Z=0 C=0

;Tests for SBC

;    00 - 00 and C=0 gives 99 and N=1 V=0 Z=0 C=0
;    00 - 00 and C=1 gives 00 and N=0 V=0 Z=1 C=1
;    00 - 01 and C=1 gives 99 and N=1 V=0 Z=0 C=0
;    0a - 00 and C=1 gives 0a and N=0 V=0 Z=0 C=1
;    0b - 00 and C=0 gives 0a and N=0 V=0 Z=0 C=1
;    9a - 00 and C=1 gives 9a and N=1 V=0 Z=0 C=1
;    9b - 00 and C=0 gives 9a and N=1 V=0 Z=0 C=1


    .SETCPU "6502"
    .SEGMENT "CODE"


TEST:   
        SED

        LDA #$00
        CLC
        ADC #$00

        LDA #$79
        SEC
        ADC #$00

        LDA #$24
        CLC
        ADC #$56

        LDA #$93
        CLC
        ADC #$82

        LDA #$89
        CLC
        ADC #$76

        LDA #$89
        SEC
        ADC #$76

        LDA #$80
        CLC
        ADC #$F0

        LDA #$80
        CLC
        ADC #$FA

        LDA #$2F
        CLC
        ADC #$4F

        LDA #$6F
        SEC
        ADC #$00


        LDA #$00
        CLC
        SBC #$00

        LDA #$00
        SEC
        SBC #$00

        LDA #$00
        SEC
        SBC #$01

        LDA #$0A
        SEC
        SBC #$00

        LDA #$0B
        CLC
        SBC #$00

        LDA #$9A
        SEC
        SBC #$00

        LDA #$9B
        CLC
        SBC #$00


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

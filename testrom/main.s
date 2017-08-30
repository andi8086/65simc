    .SETCPU "6502"
    .SEGMENT "CODE"

__init:
   
    LDA #0
    STA $00
INCREASE:
    INC $00
    BNE INCREASE
    

 
    CLD
    CLC
    LDA #$00
    ADC #$00
    SEC
    ADC #$00
    SEC
    SBC #$01
   
    CLC
    ADC #10 
    SEC
    SBC #10
    
    SED
    CLC
    ADC #9
    ADC #1
    SEC
    SBC #1
    SEC 
    SBC #9

    CLD

    JMP $FFFF


    LDA #$EA
    STA $200
    CMP $200
    BEQ equal
    JMP error
equal: 
    LDX #$33
    CPX #$33
    BNE error
    LDY #$44
    CPY $200
    BEQ error
    CLC
    ADC #$11
    CMP #$FB
    BNE error

    PHA
    LDA #$00
    PLA
    TXA
    LDX #$00
    TAX
    TYA
    LDY #$00
    TAY

    SED
    SEI
    SEC
    PHP
    PLA
    CLC
    CLI
    CLD

    LDX #$2D
    LDY #$1
    STA $1233,Y
    SEC
    SBC $1234
    BNE error

    JSR ShiftTest

    JMP $FFFF

error:
    LDA #$FF
    LDX #$FF
    LDY #$FF
    JMP $FFFF


ShiftTest:
    LDA #$FF
    ASL
    ASL
    ASL
    ASL
    ASL
    ASL
    ASL
    ASL
    CLC
    SEC
    ROR
    ROR
    ROR
    ROR
    ROR
    ROR
    ROR
    ROR
    ROR
    ROL
    ROL
    ROL
    ROL
    ROL
    ROL
    ROL
    ROL
    ROL
    LDA #$EF
    LSR
    LSR
    LSR
    LSR
    LSR
    LSR
    LSR
    LSR
    LSR
    RTS

__nmi:
    NOP
    RTI

__irq:
    NOP
    RTI

    .SEGMENT "VECTORS"
    .word __nmi
    .word __init
    .word __irq

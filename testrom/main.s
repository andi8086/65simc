    .SETCPU "6502"
    .SEGMENT "CODE"

__init:
    
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
  
    JMP $FFFF

error:
    LDA #$FF
    LDX #$FF
    LDY #$FF
    JMP $FFFF

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

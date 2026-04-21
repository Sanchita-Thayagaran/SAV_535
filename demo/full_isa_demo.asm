start:
    ADDI R1, R0, 8
    ADDI R2, R0, 3
    ADD  R3, R1, R2
    SUB  R4, R1, R2
    MUL  R5, R1, R2
    DIV  R6, R1, R2
    MOD  R7, R1, R2
    AND  R8, R1, R2
    OR   R9, R1, R2
    XOR  R10, R1, R2
    NOT  R11, R1
    SLL  R12, R1, R2
    SRL  R13, R12, R2
    SRA  R14, R12, R2
    SLT  R4, R2, R1

    ADDI R1, R0, 40
    ADDI R2, R0, 99
    SW   R2, R1, 0
    LW   R3, R1, 0

    ADDI R4, R0, 2
    ADDI R5, R0, 2
    BEQ  R4, R5, equal_label
    ADDI R6, R0, 111

equal_label:
    LA   R10, subr
    JAL  R10
    HALT

subr:
    ADDI R7, R0, 123
    RET

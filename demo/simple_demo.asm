# Simple Demo - Tests all 17 instructions without loops
# Arithmetic
ADDI R1, R0, 20
ADDI R2, R0, 7
ADD  R3, R1, R2
SUB  R4, R1, R2
MUL  R5, R2, R2
DIV  R6, R1, R2
MOD  R7, R1, R2

# Logical
ADDI R8, R0, 255
ADDI R9, R0, 15
AND  R10, R8, R9
OR   R11, R8, R9
XOR  R12, R8, R9
NOT  R13, R8

# Shifts
ADDI R1, R0, 8
ADDI R2, R0, 2
SLL  R3, R1, R2
SRL  R4, R3, R2

# Comparison
SLT  R5, R1, R8

# Memory
ADDI R1, R0, 100
ADDI R2, R0, 4660
SW   R2, R1, 0
LW   R3, R1, 0

# Branches (simple - no loops)
ADDI R1, R0, 5
ADDI R2, R0, 5
BEQ  R1, R2, skip1
ADDI R10, R0, 999

skip1:
ADDI R11, R0, 111
HALT

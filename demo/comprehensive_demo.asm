# Comprehensive Demo Program for CS 535 - April 17, 2026
# Tests ALL instruction classes for complete ISA validation

main:
    # ===== TEST 1: Arithmetic Operations =====
    ADDI R1, R0, 20      # R1 = 20
    ADDI R2, R0, 7       # R2 = 7
    
    ADD  R3, R1, R2      # R3 = 20 + 7 = 27
    SUB  R4, R1, R2      # R4 = 20 - 7 = 13
    MUL  R5, R2, R2      # R5 = 7 * 7 = 49
    DIV  R6, R1, R2      # R6 = 20 / 7 = 2
    MOD  R7, R1, R2      # R7 = 20 % 7 = 6
    
    # ===== TEST 2: Logical Operations =====
    ADDI R8, R0, 0xFF    # R8 = 255
    ADDI R9, R0, 0x0F    # R9 = 15
    
    AND  R10, R8, R9     # R10 = 0xFF & 0x0F = 0x0F
    OR   R11, R8, R9     # R11 = 0xFF | 0x0F = 0xFF
    XOR  R12, R8, R9     # R12 = 0xFF ^ 0x0F = 0xF0
    NOT  R13, R8         # R13 = ~0xFF
    
    # ===== TEST 3: Shift Operations =====
    ADDI R1, R0, 8       # R1 = 8
    ADDI R2, R0, 2       # R2 = 2
    
    SLL  R3, R1, R2      # R3 = 8 << 2 = 32
    SRL  R4, R3, R2      # R4 = 32 >> 2 = 8 (logical)
    ADDI R5, R0, -16     # R5 = -16
    SRA  R6, R5, R2      # R6 = -16 >> 2 = -4 (arithmetic)
    
    # ===== TEST 4: Comparison =====
    ADDI R1, R0, 10
    ADDI R2, R0, 20
    SLT  R3, R1, R2      # R3 = (10 < 20) = 1
    SLT  R4, R2, R1      # R4 = (20 < 10) = 0
    
    # ===== TEST 5: Memory Operations =====
    ADDI R1, R0, 100     # Memory address = 100
    ADDI R2, R0, 0x1234  # Value to store
    SW   R2, R1, 0       # Mem[100] = 0x1234
    
    ADDI R3, R0, 0       # Clear R3
    LW   R3, R1, 0       # R3 = Mem[100] = 0x1234
    
    # Store multiple values
    ADDI R4, R0, 42
    ADDI R5, R0, 99
    SW   R4, R1, 1       # Mem[101] = 42
    SW   R5, R1, 2       # Mem[102] = 99
    LW   R6, R1, 1       # R6 = 42
    LW   R7, R1, 2       # R7 = 99
    
    # ===== TEST 6: Conditional Branches =====
    ADDI R1, R0, 5
    ADDI R2, R0, 5
    BEQ  R1, R2, equal_branch
    ADDI R10, R0, 999    # Should be skipped
    
equal_branch:
    ADDI R11, R0, 111    # R11 = 111
    
    ADDI R1, R0, 3
    ADDI R2, R0, 7
    BNE  R1, R2, not_equal_branch
    ADDI R12, R0, 888    # Should be skipped
    
not_equal_branch:
    ADDI R13, R0, 222    # R13 = 222
    
    # BLT test
    ADDI R1, R0, 5
    ADDI R2, R0, 10
    BLT  R1, R2, less_than_branch
    ADDI R1, R0, 777     # Should be skipped
    
less_than_branch:
    ADDI R1, R0, 333     # R1 = 333
    
    # BGE test
    ADDI R2, R0, 10
    ADDI R3, R0, 5
    BGE  R2, R3, greater_equal_branch
    ADDI R2, R0, 666     # Should be skipped
    
greater_equal_branch:
    ADDI R2, R0, 444     # R2 = 444
    
    # ===== TEST 7: Subroutine Call =====
    LA   R10, subroutine
    JAL  R10             # Call subroutine, R15 = return address
    
    # After return
    ADDI R8, R0, 55      # R8 = 55
    
    # ===== TEST 8: Loop Test =====
    ADDI R1, R0, 0       # Counter
    ADDI R2, R0, 5       # Limit
    
loop:
    ADDI R1, R1, 1       # Increment counter
    BLT  R1, R2, loop    # Loop while R1 < 5
    
    # R1 should now be 5
    
    HALT

subroutine:
    ADDI R9, R0, 123     # R9 = 123
    RET                  # Return to caller
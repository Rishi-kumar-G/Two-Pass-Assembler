; test1_arithmetic.asm
; Tests basic arithmetic operations (add and subtract)

start:  ldc 10      ; Load constant 10
        stl 0       ; Store in local variable 0
        ldc 5       ; Load constant 5
        stl 1       ; Store in local variable 1
        ldl 0       ; Load first number
        ldl 1       ; Load second number
        add         ; Add numbers
        stl 2       ; Store result
        ldl 0       ; Load first number again
        ldl 1       ; Load second number again
        sub         ; Subtract numbers
        stl 3       ; Store result
        HALT        ; End program
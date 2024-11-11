; test4_variables.asm
; Tests variable declaration and usage

value1: SET 42      ; Define variable with value 42
value2: SET 58      ; Define variable with value 58

start:  ldc value1  ; Load value1
        stl 0       ; Store in memory
        ldc value2  ; Load value2
        stl 1       ; Store in memory
        ldl 0       ; Load first value
        ldl 1       ; Load second value
        add         ; Add them
        stl 2       ; Store result
        HALT        ; End program
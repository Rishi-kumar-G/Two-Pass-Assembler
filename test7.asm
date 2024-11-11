; test2_shifts.asm
; Tests shift left and shift right operations

        ldc 8       ; Load constant 8
        stl 0       ; Store in memory
        ldl 0       ; Load value
        shl         ; Shift left (multiply by 2)
        stl 1       ; Store shifted left result
        ldl 0       ; Load original value
        shr         ; Shift right (divide by 2)
        stl 2       ; Store shifted right result
        HALT        ; End program
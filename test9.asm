; test5_subroutine.asm
; Tests subroutine call and return

start:  ldc 10      ; Load argument
        stl 0       ; Store argument
        call func   ; Call subroutine
        HALT        ; End program

func:   ldl 0       ; Load argument
        adc 5       ; Add constant 5
        stl 1       ; Store result
        return      ; Return from subroutine
#include <avr/io.h>

.macro PORT_CONF_ENTRY port, ddr, bit
    .word _SFR_MEM_ADDR(\port)
    .word _SFR_MEM_ADDR(\ddr)
    .byte \bit
.endm

#define MATRIX_LED_WIDTH 8
#define MATRIX_LED_HEIGHT 8

.section .vectors, "ax", @progbits
.org 0x0000
    rjmp MAIN

.section .text
    .TYPE MAIN, @function
    MAIN:                                        ; Function 'MAIN':
    INITIALISE_STACK_POINTERS:
        LDI  R16, lo8(RAMEND)                   
        OUT  _SFR_IO_ADDR(SPL), R16
        LDI  R16, hi8(RAMEND)
        OUT  _SFR_IO_ADDR(SPH), R16
    
        LDI  R20, 1                              ; Load '1' to R20; will be used to shift bits.

    SET_MATRIX_LED_COL_PINS_MODE:
        LDI  R16, MATRIX_LED_WIDTH               ; Load MATRIX_LED_WIDTH to R16; will be used as loop count.

        LDI  R30, lo8(MATRIX_LED_COL_PINS)       ; Load the first address of MATRIX_LED_COL_PINS to Z register.
        LDI  R31, hi8(MATRIX_LED_COL_PINS)       

    SET_MATRIX_LED_COL_PINS_MODE_LOOP:
        LDD  R26, Z+2                            ; Store ddr to X register.
        LDD  R27, Z+3
        LD   R24, X                              ; Peek X register and store the ddr value to R24.
        LDD  R18, Z+4                            ; Store bit offset to R18.

        MOV  R22, R20                            ; Copy '1' (R20) to R22.

        TST  R18                                 ; Is bit offset (R18) zero?
        BREQ SET_COL_PIN_MODE_APPLY_MASK         ; If so, jump to SET_COL_PIN_MODE_APPLY_MASK directly.
                                                 ; If not, continue to SET_COL_PIN_MODE_SHIFT_LOOP.
    
    SET_COL_PIN_MODE_SHIFT_LOOP:
        LSL  R22                                 ; Shift one bit to the left.
        DEC  R18                                 ; Decrease bit offset (R18).
        BRNE SET_COL_PIN_MODE_SHIFT_LOOP         ; Repeat SET_COL_PIN_MODE_SHIFT_LOOP until R18 = 0.

    SET_COL_PIN_MODE_APPLY_MASK:
        OR   R24, R22                            ; DDR (R24) | 1 (R22).
        ST   X, R24                              ; Put altered DDR value (R24) back to X register.

        ADIW R30, 5                              ; Set Z register to next PORT_CONF_ENTRY.

        DEC  R16                                 ; Decrease loop count (R16).
        BREQ SET_MATRIX_LED_ROW_PINS_MODE        ; If loop count (R16) is zero, jump to SET_MATRIX_LED_ROW_PINS_MODE.
                                                 ; If not, continue.
        
        RJMP SET_MATRIX_LED_COL_PINS_MODE_LOOP   ; Back to SET_MATRIX_LED_COL_PINS_MODE_LOOP.
    
    SET_MATRIX_LED_ROW_PINS_MODE:
        LDI  R16, MATRIX_LED_HEIGHT              ; Load MATRIX_LED_HEIGHT to R16; will be used as loop count.
        
        LDI  R30, lo8(MATRIX_LED_ROW_PINS)       ; Load the first address of MATRIX_LED_ROW_PINS to Z register.
        LDI  R31, hi8(MATRIX_LED_ROW_PINS)

    SET_MATRIX_LED_ROW_PINS_MODE_LOOP:
        LDD  R26, Z+2                            ; Store ddr to X register.
        LDD  R27, Z+3
        LD   R24, X                              ; Peek X register and store the ddr value to R24.
        LDD  R18, Z+4                            ; Store bit offset to R18.

        MOV  R22, R20                            ; Copy '1' (R20) to R22.

        TST  R18                                 ; Is bit offset (R18) zero?
        BREQ SET_ROW_PIN_MODE_APPLY_MASK         ; If so, jump to SET_ROW_PIN_MODE_APPLY_MASK directly.
                                                 ; If not, continue to SET_ROW_PIN_MODE_SHIFT_LOOP.

    SET_ROW_PIN_MODE_SHIFT_LOOP:
        LSL  R22                                 ; Shift one bit to the left.
        DEC  R18                                 ; Decrease bit offset (R18).
        BRNE SET_ROW_PIN_MODE_SHIFT_LOOP         ; Repeat SET_ROW_PIN_MODE_SHIFT_LOOP until R18 = 0.

    SET_ROW_PIN_MODE_APPLY_MASK:
        OR   R24, R22                            ; DDR (R24) | 1 (R22).
        ST   X, R24                              ; Put altered DDR value (R24) back to X register.

        ADIW R30, 5                              ; Set Z register to next PORT_CONF_ENTRY.

        DEC  R16                                 ; Decrease loop count (R16).
        BREQ MAIN_LOOP                           ; If loop count (R16) is zero, jump to MAIN_LOOP.
                                                 ; If not, continue.
        
        RJMP SET_MATRIX_LED_ROW_PINS_MODE_LOOP   ; Back to SET_MATRIX_LED_ROW_PINS_MODE_LOOP.
    
    MAIN_LOOP:                                   ; TEST
        LDI  R30, lo8(MATRIX_LED_COL_PINS)       
        LDI  R31, hi8(MATRIX_LED_COL_PINS)
        LDD  R16, Z+4

        LDI  R18, 1

    TEMP_SHIFT_LOOP_1:
        LSL  R18
        DEC  R16
        BRNE TEMP_SHIFT_LOOP_1

        LD   R20, Z
        OR   R20, R18
        ST   Z, R20

        LDI  R30, lo8(MATRIX_LED_ROW_PINS)
        LDI  R31, hi8(MATRIX_LED_ROW_PINS)
        LDD  R16, Z+4

        LDI  R18, 1
    
    TEMP_SHIFT_LOOP_2:
        LSL  R18
        DEC  R16
        BRNE TEMP_SHIFT_LOOP_2

        LD   R20, Z
        OR   R20, R18
        ST   Z, R20

        RJMP MAIN_LOOP

    .TYPE MATRIX_LED_ROW_PINS, @object
    .SIZE MATRIX_LED_ROW_PINS, 40
    MATRIX_LED_ROW_PINS:
        PORT_CONF_ENTRY PORTB, DDRB, PORTB3      ; ROW 8
        PORT_CONF_ENTRY PORTB, DDRB, PORTB4      ; ROW 7
        PORT_CONF_ENTRY PORTB, DDRB, PORTB2      ; ROW 6
        PORT_CONF_ENTRY PORTB, DDRB, PORTB5      ; ROW 5
        PORT_CONF_ENTRY PORTC, DDRC, PORTC4      ; ROW 4
        PORT_CONF_ENTRY PORTB, DDRB, PORTB1      ; ROW 3
        PORT_CONF_ENTRY PORTC, DDRC, PORTC5      ; ROW 2
        PORT_CONF_ENTRY PORTD, DDRD, PORTD0      ; ROW 1

    .TYPE MATRIX_LED_COL_PINS, @object
    .SIZE MATRIX_LED_COL_PINS, 40
    MATRIX_LED_COL_PINS:
        PORT_CONF_ENTRY PORTB, DDRB, PORTB6      ; COL 1
        PORT_CONF_ENTRY PORTD, DDRD, PORTD6      ; COL 2
        PORT_CONF_ENTRY PORTD, DDRD, PORTD7      ; COL 3
        PORT_CONF_ENTRY PORTD, DDRD, PORTD1      ; COL 4
        PORT_CONF_ENTRY PORTB, DDRB, PORTB0      ; COL 5
        PORT_CONF_ENTRY PORTD, DDRD, PORTD4      ; COL 6
        PORT_CONF_ENTRY PORTB, DDRB, PORTB7      ; COL 7
        PORT_CONF_ENTRY PORTD, DDRD, PORTD5      ; COL 8

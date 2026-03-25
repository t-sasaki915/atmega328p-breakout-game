 #include <avr/io.h>

 #define PORT_CONF_ENTRY(port, ddr, bit) \
    .word _SFR_MEM_ADDR(port);           \
    .word _SFR_MEM_ADDR(ddr);            \
    .byte bit

#define MATRIX_LED_WIDTH 8
#define MATRIX_LED_HEIGHT 8

.section .vectors
    RJMP MAIN

.section .text
    .TYPE MAIN, @function
    MAIN:                                        ; Function 'MAIN':
        LDI  R16, lo8(RAMEND)                    ; Initialise stack pointers.
        OUT  _SFR_IO_ADDR(SPL), R16
        LDI  R16, hi8(RAMEND)
        OUT  _SFR_IO_ADDR(SPH), R16

                                                 ; Set MATRIX_LED_COL_PINS to output mode.

        LDI  R20, 1                              ; Load '1' to R20; will be used to shift bits.

        LDI  R16, MATRIX_LED_WIDTH               ; Load MATRIX_LED_WIDTH to R16; will be used as loop count.
        LDI  R17, MATRIX_LED_HEIGHT              ; Load MATRIX_LED_HEIGHT to R17; will be used as loop count.

        LDI  R30, lo8(MATRIX_LED_COL_PINS)       ; Load the first address of MATRIX_LED_COL_PINS to Z register.
        LDI  R31, hi8(MATRIX_LED_COL_PINS)       

    INIT_LED_DDR_LOOP:
        LDD  R26, Z+2                            ; Store ddr to X register.
        LDD  R27, Z+3
        LD   R24, X                              ; Peek X register and store the ddr value to R24.
        LDD  R18, Z+4                            ; Store bit offset to R18.

        MOV  R22, R20                            ; Copy '1' (R20) to R22.

        TST  R18                                 ; Is bit offset (R18) zero?
        BREQ INIT_LED_DDR_APPLY_MASK             ; If so, jump to INIT_LED_DDR_APPLY_MASK directly.
                                                 ; If not, continue to INIT_LED_DDR_SHIFT_LOOP.
    
    INIT_LED_DDR_SHIFT_LOOP:
        LSL  R22                                 ; Shift one bit to the left.
        DEC  R18                                 ; Decrease bit offset (R18).
        BRNE INIT_LED_DDR_SHIFT_LOOP             ; Repeat INIT_LED_DDR_SHIFT_LOOP until R18 = 0.

    INIT_LED_DDR_APPLY_MASK:
        OR   R24, R22                            ; DDR (R24) | 1 (R22).
        ST   X, R24                              ; Put altered DDR value (R24) back to X register.

        ADIW R30, 5                              ; Set Z register to next PORT_CONF_ENTRY.

        DEC  R16                                 ; Decrease loop count (R16).
        BREQ INITIALISE_CONTINUE                 ; If loop count (R16) is zero, jump to INITIALISE_CONTINUE.
                                                 ; If not, continue.
        
        RJMP  INIT_LED_DDR_LOOP                  ; Back to INIT_LED_DDR_LOOP.
    
    INITIALISE_CONTINUE:
        NOP                                      ; TODO

.section .rodata
    .TYPE MATRIX_LED_ROW_PINS, @object
    .SIZE MATRIX_LED_ROW_PINS, 40
    MATRIX_LED_ROW_PINS:
        PORT_CONF_ENTRY(PORTB, DDRB, PORTB3)     ; ROW 8
        PORT_CONF_ENTRY(PORTB, DDRB, PORTB4)     ; ROW 7
        PORT_CONF_ENTRY(PORTB, DDRB, PORTB2)     ; ROW 6
        PORT_CONF_ENTRY(PORTB, DDRB, PORTB5)     ; ROW 5
        PORT_CONF_ENTRY(PORTC, DDRC, PORTC4)     ; ROW 4
        PORT_CONF_ENTRY(PORTB, DDRB, PORTB1)     ; ROW 3
        PORT_CONF_ENTRY(PORTC, DDRC, PORTC5)     ; ROW 2
        PORT_CONF_ENTRY(PORTD, DDRD, PORTD0)     ; ROW 1

    .TYPE MATRIX_LED_COL_PINS, @object
    .SIZE MATRIX_LED_COL_PINS, 40
    MATRIX_LED_COL_PINS:
        PORT_CONF_ENTRY(PORTB, DDRB, PORTB6)     ; COL 1
        PORT_CONF_ENTRY(PORTD, DDRD, PORTD6)     ; COL 2
        PORT_CONF_ENTRY(PORTD, DDRD, PORTD7)     ; COL 3
        PORT_CONF_ENTRY(PORTD, DDRD, PORTD1)     ; COL 4
        PORT_CONF_ENTRY(PORTB, DDRB, PORTB0)     ; COL 5
        PORT_CONF_ENTRY(PORTD, DDRD, PORTD4)     ; COL 6
        PORT_CONF_ENTRY(PORTB, DDRB, PORTB7)     ; COL 7
        PORT_CONF_ENTRY(PORTD, DDRD, PORTD5)     ; COL 8

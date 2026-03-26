#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>
#include <util/delay.h>

#define TARGET_BALL_ROWS 2

#define MATRIX_LED_WIDTH 8
#define MATRIX_LED_HEIGHT 8
#define MATRIX_LED_X_MAX 7
#define MATRIX_LED_Y_MAX 7

#define BUTTON_MOVE_LEFT_PIN PORTD3
#define BUTTON_MOVE_RIGHT_PIN PORTD2

typedef struct
{
    uint8_t x;
    uint8_t y;
    int8_t dx;
    int8_t dy;
} MovingBall;

typedef struct
{
    volatile uint8_t *port;
    volatile uint8_t *ddr;
    uint8_t bit;
} PortConfig;

static const PortConfig MATRIX_LED_COL_PINS[] = {
    {&PORTB, &DDRB, PORTB6}, // COL 1
    {&PORTD, &DDRD, PORTD6}, // COL 2
    {&PORTD, &DDRD, PORTD7}, // COL 3
    {&PORTD, &DDRD, PORTD1}, // COL 4
    {&PORTB, &DDRB, PORTB0}, // COL 5
    {&PORTD, &DDRD, PORTD4}, // COL 6
    {&PORTB, &DDRB, PORTB7}, // COL 7
    {&PORTD, &DDRD, PORTD5}  // COL 8
};

static const PortConfig MATRIX_LED_ROW_PINS[] = {
    {&PORTB, &DDRB, PORTB3}, // ROW 8
    {&PORTB, &DDRB, PORTB4}, // ROW 7
    {&PORTB, &DDRB, PORTB2}, // ROW 6
    {&PORTB, &DDRB, PORTB5}, // ROW 5
    {&PORTC, &DDRC, PORTC4}, // ROW 4
    {&PORTB, &DDRB, PORTB1}, // ROW 3
    {&PORTC, &DDRC, PORTC5}, // ROW 2
    {&PORTD, &DDRD, PORTD0}  // ROW 1
};

static MovingBall MOVING_BALL = {0, 1, 1, 1};
static uint8_t TARGET_BALLS[TARGET_BALL_ROWS] = {0b11111111, 0b11111111};
static volatile uint8_t PADDLE_POSITION = 0;
static volatile uint8_t IS_GAMEOVER = 0;

static uint8_t VRAM_1[8];
static uint8_t VRAM_2[8];
static uint8_t *VRAM_TEMP = VRAM_1;
static volatile uint8_t *VRAM_DISPLAY = VRAM_2;
static volatile uint8_t CURRENT_VIEW_LINE = 0;

void MoveBall(void)
{
    uint8_t oldX = MOVING_BALL.x;
    uint8_t oldY = MOVING_BALL.y;
    int8_t newX = oldX + MOVING_BALL.dx;
    int8_t newY = oldY + MOVING_BALL.dy;
    int8_t newDX = MOVING_BALL.dx;
    int8_t newDY = MOVING_BALL.dy;
    uint8_t isChanged = 0;

    if (newY == 0)
    {
        if (oldX == PADDLE_POSITION || oldX == PADDLE_POSITION + 1)
        {
            newDY = 1;

            isChanged = 1;
        }
        else if (newX == PADDLE_POSITION || newX == PADDLE_POSITION + 1)
        {
            newDY = 1;
            newDX *= -1;

            isChanged = 1;
        }
        else
        {
            IS_GAMEOVER = 1;
        }
    }

    if (isChanged)
    {
        newX = oldX + newDX;
        newY = oldY + newDY;

        isChanged = 0;
    }

    if (newX < 0)
    {
        newDX = 1;

        isChanged = 1;
    }
    else if (newX > MATRIX_LED_X_MAX)
    {
        newDX = -1;

        isChanged = 1;
    }

    if (newY > MATRIX_LED_Y_MAX)
    {
        newDY = -1;

        isChanged = 1;
    }

    if (isChanged)
    {
        newX = oldX + newDX;
        newY = oldY + newDY;

        isChanged = 0;
    }

    if (newY >= MATRIX_LED_WIDTH - TARGET_BALL_ROWS)
    {
        if (TARGET_BALLS[MATRIX_LED_Y_MAX - newY] & (1 << newX))
        {
            TARGET_BALLS[MATRIX_LED_Y_MAX - newY] &= ~(1 << newX);

            newDY *= -1;

            isChanged = 1;
        }
    }

    if (isChanged)
    {
        newX = oldX + newDX;
        newY = oldY + newDY;

        isChanged = 0;

        uint8_t isClear = 1;
        for (uint8_t i = 0; i < TARGET_BALL_ROWS; i++)
        {
            if (TARGET_BALLS[i] != 0)
            {
                isClear = 0;

                break;
            }
        }

        IS_GAMEOVER = isClear;
    }

    MOVING_BALL.x = newX;
    MOVING_BALL.y = newY;
    MOVING_BALL.dx = newDX;
    MOVING_BALL.dy = newDY;
}

void UpdateVRAM(void)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        VRAM_TEMP[i] = 0;
    }

    VRAM_TEMP[MOVING_BALL.y] |= (1 << MOVING_BALL.x);

    VRAM_TEMP[0] |= (1 << PADDLE_POSITION) | (1 << (PADDLE_POSITION + 1));

    for (uint8_t i = 0; i < TARGET_BALL_ROWS; i++)
    {
        VRAM_TEMP[MATRIX_LED_Y_MAX - i] = TARGET_BALLS[i];
    }

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        (void)sreg_save;

        uint8_t *vramDisplayAddress = (uint8_t *)VRAM_DISPLAY;
        VRAM_DISPLAY = VRAM_TEMP;
        VRAM_TEMP = vramDisplayAddress;
    }
}

#define HIGH_PORT(portConf) *(portConf.port) |= (1 << (portConf.bit))
#define LOW_PORT(portConf) *(portConf.port) &= ~(1 << (portConf.bit))

// View timer
ISR(TIMER1_COMPA_vect)
{
    if (CURRENT_VIEW_LINE == 0)
    {
        LOW_PORT(MATRIX_LED_ROW_PINS[MATRIX_LED_Y_MAX]);
    }
    else
    {
        LOW_PORT(MATRIX_LED_ROW_PINS[CURRENT_VIEW_LINE - 1]);
    }

    for (uint8_t i = 0; i < MATRIX_LED_WIDTH; i++)
    {
        if (VRAM_DISPLAY[CURRENT_VIEW_LINE] & (1 << i))
        {
            HIGH_PORT(MATRIX_LED_COL_PINS[i]);
        }
        else
        {
            LOW_PORT(MATRIX_LED_COL_PINS[i]);
        }
    }

    HIGH_PORT(MATRIX_LED_ROW_PINS[CURRENT_VIEW_LINE]);

    if (CURRENT_VIEW_LINE == MATRIX_LED_Y_MAX)
    {
        CURRENT_VIEW_LINE = 0;
    }
    else
    {
        CURRENT_VIEW_LINE++;
    }
}

// MOVE_LEFT_SW
ISR(INT1_vect)
{
    if (IS_GAMEOVER)
    {
        return;
    }

    if (PADDLE_POSITION == 0)
    {
        return;
    }

    PADDLE_POSITION--;
}

// MOVE_RIGHT_SW
ISR(INT0_vect)
{
    if (IS_GAMEOVER)
    {
        return;
    }

    if (PADDLE_POSITION == MATRIX_LED_X_MAX - 1)
    {
        return;
    }

    PADDLE_POSITION++;
}

void InitInterruption(void)
{
    EICRA |= (1 << ISC01) | (1 << ISC11);
    EICRA &= ~((1 << ISC00) | (1 << ISC10));
    EIMSK |= (1 << INT0) | (1 << INT1);
}

void InitTimer(void)
{
    OCR1A = 124;
    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS11);
    TIMSK1 |= (1 << OCIE1A);
}

int main(void)
{
    for (uint8_t i = 0; i < MATRIX_LED_WIDTH; i++)
    {
        *MATRIX_LED_COL_PINS[i].ddr |= (1 << MATRIX_LED_COL_PINS[i].bit);
    }
    for (uint8_t i = 0; i < MATRIX_LED_HEIGHT; i++)
    {
        *MATRIX_LED_ROW_PINS[i].ddr |= (1 << MATRIX_LED_ROW_PINS[i].bit);
    }

    DDRD &= ~((1 << BUTTON_MOVE_RIGHT_PIN) | (1 << BUTTON_MOVE_LEFT_PIN));
    PORTD |= (1 << BUTTON_MOVE_RIGHT_PIN) | (1 << BUTTON_MOVE_LEFT_PIN);

    InitInterruption();
    InitTimer();

    sei();

    UpdateVRAM();

    for (;;)
    {
        if (!IS_GAMEOVER)
        {
            MoveBall();
            UpdateVRAM();
        }

        _delay_ms(10);
    }

    return 0;
}

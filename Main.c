#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#define MATRIX_LED_WIDTH 8
#define MATRIX_LED_HEIGHT 8
#define MATRIX_LED_X_MAX 7
#define MATRIX_LED_Y_MAX 7

#define BUTTON_MOVE_LEFT_PIN PORTD3
#define BUTTON_MOVE_RIGHT_PIN PORTD2

typedef struct
{
    int8_t x;
    int8_t y;
} Point;

typedef enum
{
    TOWARDS_UPRIGHT,
    TOWARDS_UPLEFT,
    TOWARDS_DOWNRIGHT,
    TOWARDS_DOWNLEFT
} Direction;

typedef struct
{
    Point movingBallLocation;
    Direction movingBallDirection;
    int8_t paddleLocation;
    Point remainingBalls[16];
} GameState;

typedef enum
{
    UPDATE_TYPE_TICK,
    UPDATE_TYPE_MOVE_PADDLE_LEFT,
    UPDATE_TYPE_MOVE_PADDLE_RIGHT
} UpdateType;

typedef struct
{
    volatile uint8_t *port;
    volatile uint8_t *ddr;
    uint8_t bit;
} PortConfig;

#define HIGH_PORT(portConf) *(portConf.port) |= (1 << (portConf.bit))

#define LOW_PORT(portConf) *(portConf.port) &= ~(1 << (portConf.bit))

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

static volatile GameState GAME_STATE;
static volatile uint8_t CURRENT_VIEW_LINE = 0;

void Initialise(void)
{
    GAME_STATE.movingBallLocation.x = 0;
    GAME_STATE.movingBallLocation.y = 1;
    GAME_STATE.movingBallDirection = TOWARDS_UPRIGHT;
    GAME_STATE.paddleLocation = 0;
    for (int i = 0; i < 16; i++)
    {
        GAME_STATE.remainingBalls[i].x = i % MATRIX_LED_WIDTH;
        GAME_STATE.remainingBalls[i].y = (int8_t)i / MATRIX_LED_WIDTH;
    }
}

void MoveBall(void)
{
    switch (GAME_STATE.movingBallDirection)
    {
        case TOWARDS_UPRIGHT: {
            if (GAME_STATE.movingBallLocation.x < 7 && GAME_STATE.movingBallLocation.y < 7)
            {
                GAME_STATE.movingBallLocation.x++;
                GAME_STATE.movingBallLocation.y++;
            }
            else if (GAME_STATE.movingBallLocation.x >= 7 && GAME_STATE.movingBallLocation.y < 7)
            {
                GAME_STATE.movingBallDirection = TOWARDS_UPLEFT;
                GAME_STATE.movingBallLocation.x--;
                GAME_STATE.movingBallLocation.y++;
            }
            else if (GAME_STATE.movingBallLocation.x < 7 && GAME_STATE.movingBallLocation.y >= 7)
            {
                GAME_STATE.movingBallDirection = TOWARDS_DOWNRIGHT;
                GAME_STATE.movingBallLocation.x++;
                GAME_STATE.movingBallLocation.y--;
            }
            else // GAME_STATE.movingBallLocation.x >= 7 && GAME_STATE.movingBallLocation.y >= 7
            {
                GAME_STATE.movingBallDirection = TOWARDS_DOWNLEFT;
                GAME_STATE.movingBallLocation.x--;
                GAME_STATE.movingBallLocation.y--;
            }

            break;
        }
        case TOWARDS_UPLEFT: {
            if (GAME_STATE.movingBallLocation.x > 0 && GAME_STATE.movingBallLocation.y < 7)
            {
                GAME_STATE.movingBallLocation.x--;
                GAME_STATE.movingBallLocation.y++;
            }
            else if (GAME_STATE.movingBallLocation.x <= 0 && GAME_STATE.movingBallLocation.y < 7)
            {
                GAME_STATE.movingBallDirection = TOWARDS_UPRIGHT;
                GAME_STATE.movingBallLocation.x++;
                GAME_STATE.movingBallLocation.y++;
            }
            else if (GAME_STATE.movingBallLocation.x > 0 && GAME_STATE.movingBallLocation.y >= 7)
            {
                GAME_STATE.movingBallDirection = TOWARDS_DOWNLEFT;
                GAME_STATE.movingBallLocation.x--;
                GAME_STATE.movingBallLocation.y--;
            }
            else // GAME_STATE.movingBallLocation.x <= 0 && GAME_STATE.movingBallLocation.y >= 7
            {
                GAME_STATE.movingBallDirection = TOWARDS_DOWNRIGHT;
                GAME_STATE.movingBallLocation.x++;
                GAME_STATE.movingBallLocation.y--;
            }

            break;
        }
        case TOWARDS_DOWNRIGHT: {
            if (GAME_STATE.movingBallLocation.x < 7 && GAME_STATE.movingBallLocation.y > 0)
            {
                GAME_STATE.movingBallLocation.x++;
                GAME_STATE.movingBallLocation.y--;
            }
            else if (GAME_STATE.movingBallLocation.x >= 7 && GAME_STATE.movingBallLocation.y > 0)
            {
                GAME_STATE.movingBallDirection = TOWARDS_DOWNLEFT;
                GAME_STATE.movingBallLocation.x--;
                GAME_STATE.movingBallLocation.y--;
            }
            else if (GAME_STATE.movingBallLocation.x < 7 && GAME_STATE.movingBallLocation.y <= 0)
            {
                GAME_STATE.movingBallDirection = TOWARDS_UPRIGHT;
                GAME_STATE.movingBallLocation.x++;
                GAME_STATE.movingBallLocation.y++;
            }
            else // GAME_STATE.movingBallLocation.x >= 7 && GAME_STATE.movingBallLocation.y <= 0
            {
                GAME_STATE.movingBallDirection = TOWARDS_UPLEFT;
                GAME_STATE.movingBallLocation.x--;
                GAME_STATE.movingBallLocation.y++;
            }

            break;
        }
        case TOWARDS_DOWNLEFT: {
            if (GAME_STATE.movingBallLocation.x > 0 && GAME_STATE.movingBallLocation.y > 0)
            {
                GAME_STATE.movingBallLocation.x--;
                GAME_STATE.movingBallLocation.y--;
            }
            else if (GAME_STATE.movingBallLocation.x <= 0 && GAME_STATE.movingBallLocation.y > 0)
            {
                GAME_STATE.movingBallDirection = TOWARDS_DOWNRIGHT;
                GAME_STATE.movingBallLocation.x++;
                GAME_STATE.movingBallLocation.y--;
            }
            else if (GAME_STATE.movingBallLocation.x > 0 && GAME_STATE.movingBallLocation.y <= 0)
            {
                GAME_STATE.movingBallDirection = TOWARDS_UPLEFT;
                GAME_STATE.movingBallLocation.x--;
                GAME_STATE.movingBallLocation.y++;
            }
            else // GAME_STATE.movingBallLocation.x <= 0 && GAME_STATE.movingBallLocation.y <= 0
            {
                GAME_STATE.movingBallDirection = TOWARDS_UPRIGHT;
                GAME_STATE.movingBallLocation.x++;
                GAME_STATE.movingBallLocation.y++;
            }

            break;
        }
    }
}

void Update(UpdateType updateType)
{
    switch (updateType)
    {
        case UPDATE_TYPE_TICK: {
            MoveBall();

            break;
        }
        case UPDATE_TYPE_MOVE_PADDLE_LEFT: {
            if (GAME_STATE.paddleLocation <= 0)
            {
                return;
            }

            GAME_STATE.paddleLocation--;

            break;
        }
        case UPDATE_TYPE_MOVE_PADDLE_RIGHT: {
            if (GAME_STATE.paddleLocation >= MATRIX_LED_X_MAX - 1)
            {
                return;
            }

            GAME_STATE.paddleLocation++;

            break;
        }
    }
}

// View timer
ISR(TIMER1_COMPA_vect)
{
    HIGH_PORT(MATRIX_LED_ROW_PINS[CURRENT_VIEW_LINE]);
    for (int i = 0; i < MATRIX_LED_WIDTH; i++)
    {
        LOW_PORT(MATRIX_LED_COL_PINS[i]);
    }

    if (CURRENT_VIEW_LINE >= MATRIX_LED_Y_MAX)
    {
        CURRENT_VIEW_LINE = 0;
    }
    else
    {
        CURRENT_VIEW_LINE++;
    }

    if (CURRENT_VIEW_LINE == GAME_STATE.movingBallLocation.y)
    {
        HIGH_PORT(MATRIX_LED_COL_PINS[GAME_STATE.movingBallLocation.x]);
    }
    if (CURRENT_VIEW_LINE == 0)
    {
        HIGH_PORT(MATRIX_LED_COL_PINS[GAME_STATE.paddleLocation]);
        HIGH_PORT(MATRIX_LED_COL_PINS[GAME_STATE.paddleLocation + 1]);
    }

    LOW_PORT(MATRIX_LED_ROW_PINS[CURRENT_VIEW_LINE]);
}

// MOVE_LEFT_SW
ISR(INT1_vect)
{
    Update(UPDATE_TYPE_MOVE_PADDLE_LEFT);
}

// MOVE_RIGHT_SW
ISR(INT0_vect)
{
    Update(UPDATE_TYPE_MOVE_PADDLE_RIGHT);
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
    for (int i = 0; i < MATRIX_LED_WIDTH; i++)
    {
        *MATRIX_LED_COL_PINS[i].ddr |= (1 << MATRIX_LED_COL_PINS[i].bit);
    }
    for (int i = 0; i < MATRIX_LED_HEIGHT; i++)
    {
        *MATRIX_LED_ROW_PINS[i].ddr |= (1 << MATRIX_LED_ROW_PINS[i].bit);
    }

    for (int i = 0; i < MATRIX_LED_HEIGHT; i++)
    {
        HIGH_PORT(MATRIX_LED_ROW_PINS[i]);
    }

    DDRD &= ~((1 << BUTTON_MOVE_RIGHT_PIN) | (1 << BUTTON_MOVE_LEFT_PIN));
    PORTD |= (1 << BUTTON_MOVE_RIGHT_PIN) | (1 << BUTTON_MOVE_LEFT_PIN);

    InitInterruption();
    InitTimer();

    sei();

    Initialise();

    for (;;)
    {
        Update(UPDATE_TYPE_TICK);
        _delay_ms(50);
    }

    return 0;
}

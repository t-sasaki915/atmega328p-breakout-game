#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#define TARGET_BALL_MAX 16

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

typedef struct
{
    uint8_t x;
    uint8_t y;
    uint8_t isActive;
} TargetBall;

typedef struct
{
    uint8_t x;
    uint8_t y;
    int8_t dx;
    int8_t dy;
} MovingBall;

typedef enum
{
    TOWARDS_UPRIGHT,
    TOWARDS_UPLEFT,
    TOWARDS_DOWNRIGHT,
    TOWARDS_DOWNLEFT
} BallDirection;

typedef enum
{
    DIRECTION_TOP,
    DIRECTION_DOWN,
    DIRECTION_LEFT,
    DIRECTION_RIGHT,
    DIRECTION_TOPRIGHT,
    DIRECTION_TOPLEFT,
    DIRECTION_DOWNRIGHT,
    DIRECTION_DOWNLEFT
} Direction;

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
static TargetBall TARGET_BALLS[TARGET_BALL_MAX];
static Point MOVING_BALL_LOCATION = {0, 1};
static BallDirection MOVING_BALL_DIRECTION = TOWARDS_UPRIGHT;
static volatile uint8_t PADDLE_POSITION = 0;
static volatile uint8_t IS_GAMEOVER = 0;

static volatile uint8_t VRAM_1[8];
static volatile uint8_t VRAM_2[8];
static volatile uint8_t *VRAM_DISPLAY = VRAM_1;
static volatile uint8_t *VRAM_TEMP = VRAM_2;
static volatile uint8_t CURRENT_VIEW_LINE = 0;

void BounceBallDirection(BallDirection *ballDirection, Direction obstacleDirection)
{
    switch (*ballDirection)
    {
        case TOWARDS_UPRIGHT: {
            switch (obstacleDirection)
            {
                case DIRECTION_RIGHT: {
                    *ballDirection = TOWARDS_UPLEFT;

                    break;
                }
                case DIRECTION_TOP: {
                    *ballDirection = TOWARDS_DOWNRIGHT;

                    break;
                }
                case DIRECTION_TOPRIGHT: {
                    *ballDirection = TOWARDS_DOWNLEFT;

                    break;
                }
                default: {
                    break;
                }
            }

            break;
        }
        case TOWARDS_UPLEFT: {
            switch (obstacleDirection)
            {
                case DIRECTION_LEFT: {
                    *ballDirection = TOWARDS_UPRIGHT;

                    break;
                }
                case DIRECTION_TOP: {
                    *ballDirection = TOWARDS_DOWNLEFT;

                    break;
                }
                case DIRECTION_TOPLEFT: {
                    *ballDirection = TOWARDS_DOWNRIGHT;

                    break;
                }
                default: {
                    break;
                }
            }

            break;
        }
        case TOWARDS_DOWNRIGHT: {
            switch (obstacleDirection)
            {
                case DIRECTION_RIGHT: {
                    *ballDirection = TOWARDS_DOWNLEFT;

                    break;
                }
                case DIRECTION_DOWN: {
                    *ballDirection = TOWARDS_UPRIGHT;

                    break;
                }
                case DIRECTION_DOWNRIGHT: {
                    *ballDirection = TOWARDS_UPLEFT;

                    break;
                }
                default: {
                    break;
                }
            }

            break;
        }
        case TOWARDS_DOWNLEFT: {
            switch (obstacleDirection)
            {
                case DIRECTION_LEFT: {
                    *ballDirection = TOWARDS_DOWNRIGHT;

                    break;
                }
                case DIRECTION_DOWN: {
                    *ballDirection = TOWARDS_UPLEFT;

                    break;
                }
                case DIRECTION_DOWNLEFT: {
                    *ballDirection = TOWARDS_UPRIGHT;

                    break;
                }
                default: {
                    break;
                }
            }

            break;
        }
    }
}

Point NextMovingBallPoint(BallDirection direction)
{
    Point newPoint;
    newPoint.x = MOVING_BALL_LOCATION.x;
    newPoint.y = MOVING_BALL_LOCATION.y;

    switch (direction)
    {
        case TOWARDS_UPRIGHT: {
            newPoint.x++;
            newPoint.y++;

            break;
        }
        case TOWARDS_UPLEFT: {
            newPoint.x--;
            newPoint.y++;

            break;
        }
        case TOWARDS_DOWNRIGHT: {
            newPoint.x++;
            newPoint.y--;

            break;
        }
        case TOWARDS_DOWNLEFT: {
            newPoint.x--;
            newPoint.y--;

            break;
        }
    }

    return newPoint;
}

void MoveBall(void)
{
    BallDirection newBallDirection = MOVING_BALL_DIRECTION;
    Point currentPoint = MOVING_BALL_LOCATION;
    Point newPoint = NextMovingBallPoint(newBallDirection);

    uint8_t isBounced = 0;

    if (newPoint.y == 0)
    {
        if (currentPoint.x == PADDLE_POSITION || currentPoint.x == PADDLE_POSITION + 1)
        {
            BounceBallDirection(&newBallDirection, DIRECTION_DOWN);

            isBounced = 1;
        }
        else if (newPoint.x == PADDLE_POSITION || newPoint.x == PADDLE_POSITION + 1)
        {
            if (newBallDirection == TOWARDS_DOWNLEFT)
            {
                BounceBallDirection(&newBallDirection, DIRECTION_DOWNLEFT);

                isBounced = 1;
            }
            else if (newBallDirection == TOWARDS_DOWNRIGHT)
            {
                BounceBallDirection(&newBallDirection, DIRECTION_DOWNRIGHT);

                isBounced = 1;
            }
        }
        else
        {
            IS_GAMEOVER = 1;
        }
    }

    if (isBounced != 0)
    {
        newPoint = NextMovingBallPoint(newBallDirection);
    }

    if (newPoint.y > MATRIX_LED_Y_MAX)
    {
        if (newPoint.x > MATRIX_LED_X_MAX)
        {
            BounceBallDirection(&newBallDirection, DIRECTION_TOPRIGHT);

            isBounced = 1;
        }
        else if (newPoint.x < 0)
        {
            BounceBallDirection(&newBallDirection, DIRECTION_TOPLEFT);

            isBounced = 1;
        }
        else
        {
            BounceBallDirection(&newBallDirection, DIRECTION_TOP);

            isBounced = 1;
        }
    }
    else if (newPoint.y < 0)
    {
        if (newPoint.x > MATRIX_LED_X_MAX)
        {
            BounceBallDirection(&newBallDirection, DIRECTION_DOWNRIGHT);

            isBounced = 1;
        }
        else if (newPoint.x < 0)
        {
            BounceBallDirection(&newBallDirection, DIRECTION_DOWNLEFT);

            isBounced = 1;
        }
        else
        {
            BounceBallDirection(&newBallDirection, DIRECTION_DOWN);

            isBounced = 1;
        }
    }
    else
    {
        if (newPoint.x > MATRIX_LED_X_MAX)
        {
            BounceBallDirection(&newBallDirection, DIRECTION_RIGHT);

            isBounced = 1;
        }
        else if (newPoint.x < 0)
        {
            BounceBallDirection(&newBallDirection, DIRECTION_LEFT);

            isBounced = 1;
        }
    }

    if (isBounced != 0)
    {
        newPoint = NextMovingBallPoint(newBallDirection);
    }

    MOVING_BALL_LOCATION = newPoint;
    MOVING_BALL_DIRECTION = newBallDirection;
}

void UpdateVRAM(void)
{
    for (int i = 0; i < 8; i++)
    {
        VRAM_TEMP[i] = 0;
    }

    VRAM_TEMP[MOVING_BALL.y] |= (1 << MOVING_BALL.x);

    VRAM_TEMP[0] |= (1 << PADDLE_POSITION) | (1 << (PADDLE_POSITION + 1));

    for (int i = 0; i < TARGET_BALL_MAX; i++)
    {
        if (TARGET_BALLS[i].isActive)
        {
            VRAM_TEMP[TARGET_BALLS[i].y] |= (1 << TARGET_BALLS[i].x);
        }
    }

    volatile uint8_t *temp = VRAM_DISPLAY;
    VRAM_DISPLAY = VRAM_TEMP;
    VRAM_TEMP = temp;
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

    for (int i = 0; i < MATRIX_LED_WIDTH; i++)
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

    if (CURRENT_VIEW_LINE >= MATRIX_LED_Y_MAX)
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

    if (PADDLE_POSITION <= 0)
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

    if (PADDLE_POSITION >= MATRIX_LED_X_MAX - 1)
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
    for (int i = 0; i < MATRIX_LED_WIDTH; i++)
    {
        *MATRIX_LED_COL_PINS[i].ddr |= (1 << MATRIX_LED_COL_PINS[i].bit);
    }
    for (int i = 0; i < MATRIX_LED_HEIGHT; i++)
    {
        *MATRIX_LED_ROW_PINS[i].ddr |= (1 << MATRIX_LED_ROW_PINS[i].bit);
    }

    DDRD &= ~((1 << BUTTON_MOVE_RIGHT_PIN) | (1 << BUTTON_MOVE_LEFT_PIN));
    PORTD |= (1 << BUTTON_MOVE_RIGHT_PIN) | (1 << BUTTON_MOVE_LEFT_PIN);

    InitInterruption();
    InitTimer();

    sei();

    for (int i = 0; i < TARGET_BALL_MAX; i++)
    {
        TARGET_BALLS[i].x = i % MATRIX_LED_WIDTH;
        TARGET_BALLS[i].y = MATRIX_LED_Y_MAX - ((int8_t)i / MATRIX_LED_WIDTH);
        TARGET_BALLS[i].isActive = 1;
    }

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

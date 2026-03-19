#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#define MATRIX_LED_WIDTH 8
#define MATRIX_LED_HEIGHT 8

#define BUTTON_MOVE_LEFT_PIN PORTD3
#define BUTTON_MOVE_RIGHT_PIN PORTD2

#define MATRIX_LED_ROW_1_PIN PORTB0
#define MATRIX_LED_ROW_2_PIN PORTB1
#define MATRIX_LED_ROW_3_PIN PORTB2
#define MATRIX_LED_ROW_4_PIN PORTB3
#define MATRIX_LED_ROW_5_PIN PORTB4
#define MATRIX_LED_ROW_6_PIN PORTB5
#define MATRIX_LED_ROW_7_PIN PORTB6
#define MATRIX_LED_ROW_8_PIN PORTB7

#define MATRIX_LED_COL_1_PIN PORTD0
#define MATRIX_LED_COL_2_PIN PORTD1
#define MATRIX_LED_COL_3_PIN PORTD2
#define MATRIX_LED_COL_4_PIN PORTD3
#define MATRIX_LED_COL_5_PIN PORTD4
#define MATRIX_LED_COL_6_PIN PORTD5
#define MATRIX_LED_COL_7_PIN PORTD6
#define MATRIX_LED_COL_8_PIN PORTD7

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

static volatile GameState GAME_STATE;

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

void MoveBall()
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
            if (GAME_STATE.paddleLocation >= MATRIX_LED_WIDTH - 1)
            {
                return;
            }

            GAME_STATE.paddleLocation++;

            break;
        }
    }
}

void View(void)
{
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

int main(void)
{
    DDRB = 0xFF;
    DDRD = 0xFF;
    DDRD &= ~((1 << BUTTON_MOVE_LEFT_PIN) | (1 << BUTTON_MOVE_RIGHT_PIN));
    PORTD |= (1 << BUTTON_MOVE_LEFT_PIN) | (1 << BUTTON_MOVE_RIGHT_PIN);

    EICRA |= (1 << ISC01) | (1 << ISC11);
    EICRA &= ~((1 << ISC00) | (1 << ISC10));

    EIMSK |= (1 << INT0) | (1 << INT1);

    sei();

    Initialise();

    View();

    for (;;)
    {
        Update(UPDATE_TYPE_TICK);

        View();

        _delay_ms(5);
    }

    return 0;
}

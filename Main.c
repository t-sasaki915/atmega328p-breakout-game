#include <avr/io.h>
#include <util/delay.h>

#define MATRIX_LED_WIDTH 8
#define MATRIX_LED_HEIGHT 8

#define BUTTON_MOVE_LEFT_PIN PORTC0
#define BUTTON_MOVE_RIGHT_PIN PORTC1

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

void Initialise(GameState *gameState)
{
    gameState->movingBallLocation.x = 6;
    gameState->movingBallLocation.y = 0;
    gameState->movingBallDirection = TOWARDS_UPRIGHT;
    gameState->paddleLocation = 0;
    for (int i = 0; i < 16; i++)
    {
        gameState->remainingBalls[i].x = i % MATRIX_LED_WIDTH;
        gameState->remainingBalls[i].y = (int8_t)i / MATRIX_LED_WIDTH;
    }
}

void MovePoint(Direction *direction, Point *point)
{
    switch (*direction)
    {
        case TOWARDS_UPRIGHT: {
            if (point->x < 7 && point->y < 7)
            {
                point->x++;
                point->y++;
            }
            else if (point->x >= 7 && point->y < 7)
            {
                *direction = TOWARDS_UPLEFT;
                point->x--;
                point->y++;
            }
            else if (point->x < 7 && point->y >= 7)
            {
                *direction = TOWARDS_DOWNRIGHT;
                point->x++;
                point->y--;
            }
            else // point->x >= 7 && point->y >= 7
            {
                *direction = TOWARDS_DOWNLEFT;
                point->x--;
                point->y--;
            }

            break;
        }
        case TOWARDS_UPLEFT: {
            if (point->x > 0 && point->y < 7)
            {
                point->x--;
                point->y++;
            }
            else if (point->x <= 0 && point->y < 7)
            {
                *direction = TOWARDS_UPRIGHT;
                point->x++;
                point->y++;
            }
            else if (point->x > 0 && point->y >= 7)
            {
                *direction = TOWARDS_DOWNLEFT;
                point->x--;
                point->y--;
            }
            else // point->x <= 0 && point->y >= 7
            {
                *direction = TOWARDS_DOWNRIGHT;
                point->x++;
                point->y--;
            }

            break;
        }
        case TOWARDS_DOWNRIGHT: {
            if (point->x < 7 && point->y > 0)
            {
                point->x++;
                point->y--;
            }
            else if (point->x >= 7 && point->y > 0)
            {
                *direction = TOWARDS_DOWNLEFT;
                point->x--;
                point->y--;
            }
            else if (point->x < 7 && point->y <= 0)
            {
                *direction = TOWARDS_UPRIGHT;
                point->x++;
                point->y++;
            }
            else // point->x >= 7 && point->y <= 0
            {
                *direction = TOWARDS_UPLEFT;
                point->x--;
                point->y++;
            }

            break;
        }
        case TOWARDS_DOWNLEFT: {
            if (point->x > 0 && point->y > 0)
            {
                point->x--;
                point->y--;
            }
            else if (point->x <= 0 && point->y > 0)
            {
                *direction = TOWARDS_DOWNRIGHT;
                point->x++;
                point->y--;
            }
            else if (point->x > 0 && point->y <= 0)
            {
                *direction = TOWARDS_UPLEFT;
                point->x--;
                point->y++;
            }
            else // point->x <= 0 && point->y <= 0
            {
                *direction = TOWARDS_UPRIGHT;
                point->x++;
                point->y++;
            }

            break;
        }
    }
}

void Update(UpdateType updateType, GameState *gameState)
{
    switch (updateType)
    {
        case UPDATE_TYPE_TICK: {

            break;
        }
        case UPDATE_TYPE_MOVE_PADDLE_LEFT: {
            if (gameState->paddleLocation <= 0)
            {
                return;
            }

            gameState->paddleLocation--;

            break;
        }
        case UPDATE_TYPE_MOVE_PADDLE_RIGHT: {
            if (gameState->paddleLocation >= MATRIX_LED_WIDTH - 1)
            {
                return;
            }

            gameState->paddleLocation++;

            break;
        }
    }
}

void View(GameState *gameState)
{
}

int main(void)
{
    // PORTB output
    DDRB = 0xFF;
    // PORTC output
    DDRD = 0xFF;
    // BUTTON_MOVE_LEFT_PIN BUTTON_MOVE_RIGHT_PIN input
    DDRC &= ~(1 << BUTTON_MOVE_LEFT_PIN) | ~(1 << BUTTON_MOVE_RIGHT_PIN);
    PORTC |= (1 << BUTTON_MOVE_LEFT_PIN) | (1 << BUTTON_MOVE_RIGHT_PIN);

    GameState currentGameState;
    Initialise(&currentGameState);

    for (;;)
    {
        View(&currentGameState);
    }

    return 0;
}

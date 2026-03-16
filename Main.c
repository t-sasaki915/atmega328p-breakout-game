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

void RenderGameState(GameState *gameState)
{
}

void MovePoint(Direction direction, Point point, Direction *newDirection, Point *newPoint)
{
    switch (direction)
    {
        case TOWARDS_UPRIGHT: {
            break;
        }
        case TOWARDS_UPLEFT: {
            break;
        }
        case TOWARDS_DOWNRIGHT: {
            break;
        }
        case TOWARDS_DOWNLEFT: {
            break;
        }
    }
}

void UpdateGameState(UpdateType updateType, GameState *gameState)
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
    currentGameState.movingBallLocation.x = 6;
    currentGameState.movingBallLocation.y = 0;
    currentGameState.movingBallDirection = TOWARDS_UPRIGHT;
    currentGameState.paddleLocation = 0;
    for (int i = 0; i < 16; i++)
    {
        currentGameState.remainingBalls[i].x = i % MATRIX_LED_WIDTH;
        currentGameState.remainingBalls[i].y = (int8_t)i / MATRIX_LED_WIDTH;
    }

    for (;;)
    {
        RenderGameState(&currentGameState);
    }

    return 0;
}

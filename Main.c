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

typedef struct
{
    Point movingBallLocation;
    int8_t paddleLocation;
    Point remainingBalls[16];
} GameState;

void RenderGameState(GameState gameState)
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
    currentGameState.movingBallLocation.x = 6;
    currentGameState.movingBallLocation.y = 0;
    currentGameState.paddleLocation = 0;
    for (int i = 0; i < 16; i++)
    {
        currentGameState.remainingBalls[i].x = i % MATRIX_LED_WIDTH;
        currentGameState.remainingBalls[i].y = (int8_t)i / MATRIX_LED_WIDTH;
    }

    for (;;)
    {
        RenderGameState(currentGameState);
    }

    return 0;
}

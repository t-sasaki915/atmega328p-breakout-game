#include <avr/io.h>
#include <util/delay.h>

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

int main(void)
{
    GameState currentGameState;
    currentGameState.movingBallLocation.x = 6;
    currentGameState.movingBallLocation.y = 0;
    currentGameState.paddleLocation = 0;
    for (int i = 0; i < 16; i++)
    {
        currentGameState.remainingBalls[i].x = i % 8;
        currentGameState.remainingBalls[i].y = (int8_t)i / 8;
    }

    for (;;)
    {
    }

    return 0;
}

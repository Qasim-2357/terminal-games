#ifndef SNAKE_LOGIC_H
#define SNAKE_LOGIC_H

#include <random>
#include <vector>

struct Point
{
    int x;
    int y;
};

enum class Direction
{
    up,
    down,
    left,
    right
};

Direction directionForInput(Direction current, char input);
Point nextPoint(Point head, Direction direction);
bool isCollision(Point nextHead, const std::vector<Point>& snake,
                 bool growing);
bool spawnFood(int boardWidth, int boardHeight,
               const std::vector<Point>& snake, std::mt19937& generator,
               Point& food);
int movementIntervalMilliseconds(int score);

#endif

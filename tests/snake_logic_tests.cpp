#include "snake_logic.h"

#include <cassert>
#include <random>
#include <vector>

namespace
{
void testDirectionRules()
{
    assert(directionForInput(Direction::right, 'w') == Direction::up);
    assert(directionForInput(Direction::right, 'a') == Direction::right);
    assert(directionForInput(Direction::up, 's') == Direction::up);
    assert(directionForInput(Direction::left, 'D') == Direction::left);
    assert(directionForInput(Direction::right, 'x') == Direction::right);
}

void testMovementAndCollision()
{
    const std::vector<Point> snake = {{1, 2}, {2, 2}, {3, 2}};
    assert(nextPoint(snake.back(), Direction::up).y == 1);
    assert(!isCollision({1, 2}, snake, false));
    assert(isCollision({1, 2}, snake, true));
    assert(isCollision({2, 2}, snake, false));
    assert(isCollision({0, 2}, snake, true) == false);
    assert(!isCollision({4, 2}, snake, false));
}

void testFoodPlacementAndExhaustion()
{
    std::mt19937 generator(7);
    Point food = {-1, -1};
    const std::vector<Point> snake = {{0, 0}, {1, 0}};
    assert(spawnFood(3, 1, snake, generator, food));
    assert(food.x == 2 && food.y == 0);

    const std::vector<Point> fullBoard = {{0, 0}, {1, 0}, {2, 0}};
    assert(!spawnFood(3, 1, fullBoard, generator, food));
}

void testSmallAndInvalidBoards()
{
    std::mt19937 generator(9);
    Point food = {-1, -1};
    assert(spawnFood(1, 1, {}, generator, food));
    assert(food.x == 0 && food.y == 0);
    assert(!spawnFood(0, 1, {}, generator, food));
    assert(!spawnFood(1, 0, {}, generator, food));
}

void testSpeedProgression()
{
    assert(movementIntervalMilliseconds(0) == 90);
    assert(movementIntervalMilliseconds(10) == 60);
    assert(movementIntervalMilliseconds(100) == 45);
    assert(movementIntervalMilliseconds(-1) == 90);
}
}

int main()
{
    testDirectionRules();
    testMovementAndCollision();
    testFoodPlacementAndExhaustion();
    testSmallAndInvalidBoards();
    testSpeedProgression();
    return 0;
}

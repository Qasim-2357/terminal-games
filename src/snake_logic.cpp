#include "snake_logic.h"

#include <algorithm>

namespace
{
bool isOpposite(Direction first, Direction second)
{
    return (first == Direction::up && second == Direction::down) ||
           (first == Direction::down && second == Direction::up) ||
           (first == Direction::left && second == Direction::right) ||
           (first == Direction::right && second == Direction::left);
}

bool occupies(const std::vector<Point>& snake, Point position,
              std::size_t count)
{
    return std::any_of(
        snake.begin(), snake.begin() + static_cast<std::ptrdiff_t>(count),
        [&](const Point& part)
        {
            return part.x == position.x && part.y == position.y;
        });
}
}

Direction directionForInput(Direction current, char input)
{
    Direction requested = current;
    if (input == 'w' || input == 'W')
    {
        requested = Direction::up;
    }
    else if (input == 's' || input == 'S')
    {
        requested = Direction::down;
    }
    else if (input == 'a' || input == 'A')
    {
        requested = Direction::left;
    }
    else if (input == 'd' || input == 'D')
    {
        requested = Direction::right;
    }

    return isOpposite(current, requested) ? current : requested;
}

Point nextPoint(Point head, Direction direction)
{
    if (direction == Direction::up)
    {
        --head.y;
    }
    else if (direction == Direction::down)
    {
        ++head.y;
    }
    else if (direction == Direction::left)
    {
        --head.x;
    }
    else
    {
        ++head.x;
    }
    return head;
}

bool isCollision(Point nextHead, const std::vector<Point>& snake,
                 bool growing)
{
    if (snake.empty())
    {
        return false;
    }

    const auto firstCheckedSegment = growing ? snake.begin() : snake.begin() + 1;
    return std::any_of(
        firstCheckedSegment, snake.end(),
        [&](const Point& part)
        {
            return part.x == nextHead.x && part.y == nextHead.y;
        });
}

bool spawnFood(int boardWidth, int boardHeight,
               const std::vector<Point>& snake, std::mt19937& generator,
               Point& food)
{
    if (boardWidth <= 0 || boardHeight <= 0 ||
        snake.size() >= static_cast<std::size_t>(boardWidth * boardHeight))
    {
        return false;
    }

    std::vector<Point> available;
    available.reserve(static_cast<std::size_t>(boardWidth * boardHeight) -
                      snake.size());
    for (int y = 0; y < boardHeight; ++y)
    {
        for (int x = 0; x < boardWidth; ++x)
        {
            Point candidate = {x, y};
            if (!occupies(snake, candidate, snake.size()))
            {
                available.push_back(candidate);
            }
        }
    }

    std::shuffle(available.begin(), available.end(), generator);
    food = available.front();
    return true;
}

int movementIntervalMilliseconds(int score)
{
    const int safeScore = (std::max)(0, score);
    return (std::max)(45, 90 - safeScore * 3);
}

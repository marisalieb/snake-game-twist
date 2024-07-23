#include <iostream>
#include <raylib.h>
#include <deque>
#include <raymath.h>

using namespace std;

Color lilac = {213, 200, 230, 255};
Color darkGrey = {80, 76, 84, 255};

int cellSize = 30;
int cellCount = 25;
int offset = 75; // for border of game

// slow down snake so its not moving 60 cells per second since 60fps
double lastUpdateTime = 0;
bool eventTriggered(double interval)
{
    double currentTime = GetTime();
    if (currentTime - lastUpdateTime >= interval)
    {
        // so if interval has passed since last update
        lastUpdateTime = currentTime;
        return true;
    }

    return false;
}

bool elementInDeque(Vector2 element, deque<Vector2> deque)
{
    for (unsigned int i = 0; i < deque.size(); i++)
    {
        if (Vector2Equals(deque[i], element))
        {
            return true;
        }
    }
    return false;
}

class Snake
{
public:
    // start point and start direction
    deque<Vector2> startPoint = {Vector2{6, 12}, Vector2{5, 12}, Vector2{4, 12}}; // for 25 board
    // deque<Vector2> startPoint = {Vector2{2, 0}, Vector2{1, 0}, Vector2{0, 0}}; // for smaller window tests

    deque<Vector2> body = startPoint;
    Vector2 direction = {1, 0};

    bool addSegment = false;
    bool removeSegments = false;

    void Draw()
    {
        for (unsigned int i = 0; i < body.size(); i++)
        {
            float x = body[i].x;
            float y = body[i].y;

            DrawCircleGradient(offset + x * cellSize + 15, offset + y * cellSize + 15, (float)cellSize / 2, lilac, darkGrey);
        }
    }

    void removeSegmentsFromBack(int count)
    {
        for (int i = 0; i < count; ++i)
        {
            body.pop_back();
        }
    }

    void Update()
    {

        // remove segments when bomb is hit
        if (removeSegments == true)
        {
            removeSegmentsFromBack(2);
            removeSegments = false;
        }
        // add segment at front
        body.push_front(Vector2Add(body[0], direction)); // this will be used either way in if and in else
        if (addSegment == true)
        {
            addSegment = false;
        }

        else
        {
            // remove item at back of snake
            removeSegmentsFromBack(1);
            ;
        }
    }

    // reset snake to start after hitting wall or obstacle
    void Reset()
    {
        body = startPoint;
        direction = {1, 0};
    }

    // getter function to access the body
    deque<Vector2> &getBody()
    {
        return body;
    }
};

class Food
{
public:
    Vector2 position;
    Snake &snake;

    Food(Snake &snakeRef) : snake(snakeRef)
    {
        position = GenerateRandomPosition();
    }

    void Draw()
    {
        DrawCircleGradient(offset + position.x * cellSize + 15, offset + position.y * cellSize + 15, (float)cellSize / 2, lilac, darkGrey);
    }

    Vector2 GenerateRandomCell()
    {
        // random number from 0 to 24 for 25 spots on grid
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);
        return Vector2{x, y};
    }

    // check that new position isnt where the snake already is
    Vector2 GenerateRandomPosition()
    {

        Vector2 position = GenerateRandomCell();
        while (elementInDeque(position, snake.getBody()))
        {
            position = GenerateRandomCell();
        }
        return position;
    }
};

class Obstacle
{
public:
    Vector2 position;
    Snake &snake;
    Food &food;

    Obstacle(Snake &snakeRef, Food &foodRef) : snake(snakeRef), food(foodRef)
    {
        position = GenerateRandomPosition();
    }

    void Draw()
    {
        Rectangle segment = Rectangle{offset + position.x * cellSize, offset + position.y * cellSize, (float)cellSize, (float)cellSize};
        DrawRectangleRounded(segment, 0.25, 9, darkGrey);
    }

    Vector2 GenerateRandomCell()
    {
        // random number from 0 to 24 for 25 spots on grid
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);
        return Vector2{x, y};
    }

    // check that new position isnt where the snake or food already is
    Vector2 GenerateRandomPosition()
    {

        Vector2 position = GenerateRandomCell();
        while (elementInDeque(position, snake.getBody()) || Vector2Equals(position, food.position))
        {
            position = GenerateRandomCell();
        }
        return position;
    }
};

class Bomb
{
public:
    Vector2 position;
    Texture2D texture;

    Snake &snake;
    Food &food;
    Obstacle &obstacle;

    Bomb(Snake &snakeRef, Food &foodRef, Obstacle &obstacleRef) : snake(snakeRef), food(foodRef), obstacle(obstacleRef)
    {
        Image image = LoadImage("graphics/explosion-small.png");
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
        position = GenerateRandomPosition();
    }

    // deconstructor
    ~Bomb()
    {
        UnloadTexture(texture);
    }

    void Draw()
    {
        DrawTexture(texture, offset + position.x * cellSize, offset + position.y * cellSize, WHITE); // white here just means no colour tinting on top of image
    }

    Vector2 GenerateRandomCell()
    {
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);
        return Vector2{x, y};
    }

    // check that new position isnt where the snake, food or obstacle already is
    Vector2 GenerateRandomPosition()
    {

        Vector2 position = GenerateRandomCell();
        while (elementInDeque(position, snake.getBody()) || Vector2Equals(position, food.position) || Vector2Equals(position, obstacle.position))
        {
            position = GenerateRandomCell();
        }
        return position;
    }
};

class Game
{
public:
    Snake snake;
    Food food;
    Obstacle obstacle;
    Bomb bomb;

    bool running = true;
    int level = 0;

    Game() : snake(), food(snake), obstacle(snake, food), bomb(snake, food, obstacle) {}

    Snake &getSnake()
    {
        return snake;
    }

    void Draw()
    {
        snake.Draw();
        food.Draw();
        obstacle.Draw();
        bomb.Draw();
    }

    void Update()
    {
        if (running)
        {
            snake.Update();
            CheckCollisionFood();
            CheckCollisionEdges();
            CheckCollisionTail();
            CheckCollisionObstacle();
            CheckCollisionBomb();
        }
    }

    void CheckCollisionFood()
    {
        if (Vector2Equals(snake.getBody()[0], food.position))
        {
            // change food position after collision with snake
            food.position = food.GenerateRandomPosition();
            snake.addSegment = true;
            level++;
        }
    }

    void CheckCollisionObstacle()
    {
        if (Vector2Equals(snake.getBody()[0], obstacle.position))
        {
            GameOver();
        }
    }

    void CheckCollisionBomb()
    {
        if (Vector2Equals(snake.getBody()[0], bomb.position) && snake.body.size() >= 3)
        {
            // change bomb position after collision with snake
            bomb.position = bomb.GenerateRandomPosition();
            snake.removeSegments = true;
            level++;
        }

        if (Vector2Equals(snake.getBody()[0], bomb.position) && snake.getBody().size() < 3)
        {
            GameOver();
        }
    }

    void CheckCollisionEdges()
    {
        if (snake.getBody()[0].x == cellCount || snake.getBody()[0].x == -1)
        {
            GameOver();
        }

        if (snake.getBody()[0].y == cellCount || snake.getBody()[0].y == -1)
        {
            GameOver();
        }
    }

    void GameOver()
    {
        snake.Reset();
        food.position = food.GenerateRandomPosition();
        obstacle.position = obstacle.GenerateRandomPosition();
        bomb.position = bomb.GenerateRandomPosition();

        running = false;
        level = 0;
    }

    void CheckCollisionTail()
    {
        deque<Vector2> headlessBody = snake.getBody();
        headlessBody.pop_front();
        if (elementInDeque(snake.getBody()[0], headlessBody))
        {
            GameOver();
        }
    }
};

int main()
{

    InitWindow(2 * offset + cellSize * cellCount, 2 * offset + cellSize * cellCount, "Snake with a twist");
    SetTargetFPS(60);

    Game game;

    // game loop
    while (WindowShouldClose() == false)
    {
        BeginDrawing();

        // so every .2 seconds snake moves, can slow down for more difficult games later
        if (eventTriggered(0.2))
        {
            game.Update();
        }

        // get user input via keys
        // and add check so snake cant go backwards and immediately forwards, so the opposite direction for each one
        if (IsKeyPressed(KEY_UP) && game.snake.direction.y != 1)
        {
            game.snake.direction = {0, -1}; // since 0, 0 is in upper left corner, use -1 to move up
            game.running = true;
        }

        if (IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1)
        {
            game.snake.direction = {0, 1};
            game.running = true;
        }

        if (IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1)
        {
            game.snake.direction = {1, 0};
            game.running = true;
        }

        if (IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1)
        {
            game.snake.direction = {-1, 0};
            game.running = true;
        }

        // drawing background and rectangle around game
        ClearBackground(lilac);
        DrawRectangleLinesEx(Rectangle{(float)offset - 5, (float)offset - 5, (float)cellSize * cellCount + 10, (float)cellSize * cellCount + 10}, 5, darkGrey);

        // add game title and level
        DrawText("Snake with a Twist", offset - 5, 20, 35, darkGrey);
        DrawText(TextFormat("%i", game.level), offset - 18 + cellSize * cellCount, offset + cellSize * cellCount + 10, 45, darkGrey);

        game.Draw();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
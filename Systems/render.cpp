#include "render.h"
#include "movement.h"
#include <iostream>
#include <random>
#include <vector>
#include <chrono>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#endif

// Number of enemies to spawn per round.
static const int targetEnemyCount = 4;
// Number of enemies currently spawned.
static int enemyCounter = 0;
static bool allEnemiesSpawned = false;
// false if the enemy corresponding to that element is dead.
static bool enemyActivity[targetEnemyCount] = {false, false, false, false};
// All enemy positions are stored here: [0][] = x, [1][] = y.
static int enemyXY[2][targetEnemyCount];

static std::random_device rd;
static std::mt19937 engine(rd());

static int randomYvalue()
{
    return std::uniform_int_distribution<int>(2, 13)(engine);
}

// If all enemies are active, this will move them forward once.
// Called once per frame; updates until enemies reach their max point.
static void updateEnemies(std::vector<int>& yPositions, int enemyX)
{
    if (!allEnemiesSpawned)
    {
        return;
    }

    for (int i = 0; i < targetEnemyCount; i++)
    {
        int prevEnemyX = enemyX;
        enemyX--;

        // Remove old enemy icon.
        setCursorPos(prevEnemyX, yPositions[i]);
        std::cout << ' ';
        // Draw enemy at its new position.
        setCursorPos(enemyX, yPositions[i]);
        std::cout << enemyIcon;
    }
    movePlayer();
    std::this_thread::sleep_for(std::chrono::seconds(2));
}

// Once at the start of a round, enemies are spawned at a random y position.
// The xy position of each enemy is stored along with its active status.
// Called once per frame until all enemies have been spawned.
static void initiateEnemySpawn(int numberOfEnemies)
{
    (void)numberOfEnemies;

    if (allEnemiesSpawned)
    {
        return;
    }

    int enemyY = randomYvalue();

    setCursorPos(maxX, enemyY);
    std::cout << enemyIcon;

    enemyXY[0][enemyCounter] = maxX; // All enemies start at maxX.
    enemyXY[1][enemyCounter] = enemyY;
    enemyActivity[enemyCounter] = true;
    enemyCounter++;

    if (enemyCounter == targetEnemyCount)
    {
        allEnemiesSpawned = true;
    }
}


// Sets cursor position for drawing elements.
void setCursorPos(int x, int y)
{
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = {(short)x, (short)y};
    SetConsoleCursorPosition(hConsole, pos);
#else
    std::cout << "\033[" << (y + 1) << ";" << (x + 1) << "H" << std::flush;
#endif
}

static void setColor(int colorCode)
{
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, colorCode);
#else
    if (colorCode == 11)
    {
        std::cout << "\033[36m";
    }
    else
    {
        std::cout << "\033[0m";
    }
#endif
}

void renderBorder(int width, int height)
{
    // 11 = cyan.
    setColor(11);

    // Draw top/bottom borders.
    for (int x = 1; x < width; ++x)
    {
        setCursorPos(x, 0);
        std::cout << '_';
        setCursorPos(x, height - 1);
        std::cout << '_';
    }

    // Draw left/right borders.
    for (int y = 0; y < height - 1; ++y)
    {
        setCursorPos(0, y + 1);
        std::cout << '|';
        setCursorPos(width, y + 1);
        std::cout << '|';
    }
    setColor(7);
}

void render()
{
    if (prevPlayerX != playerX || prevPlayerY != playerY)
    {
        setCursorPos(prevPlayerX, prevPlayerY);
        std::cout << ' ';
    }

    if (bulletPrevX != bulletX || bulletPrevY != bulletY)
    {
        if (bulletPrevX >= 1 && bulletPrevX <= 99 && bulletPrevY >= 1 && bulletPrevY <= 13)
        {
            setCursorPos(bulletPrevX, bulletPrevY);
            std::cout << ' ';
        }
    }

    if (bulletActive)
    {
        setCursorPos(bulletX, bulletY);
        std::cout << '*';
    }

    setCursorPos(playerX, playerY);
    std::cout << playerIcon;
    setCursorPos(0, 16);

    movePlayer();
    renderEnemy();
}

void renderEnemy()
{
    static int currentEnemyX = maxX;

    initiateEnemySpawn(targetEnemyCount);

    if (allEnemiesSpawned)
    {
        std::vector<int> yPositions(enemyXY[1], enemyXY[1] + targetEnemyCount);
        updateEnemies(yPositions, currentEnemyX);
        currentEnemyX--;
    }
}

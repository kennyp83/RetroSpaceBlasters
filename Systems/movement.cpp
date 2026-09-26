#include "movement.h"
#include "render.h"
#include <chrono>
#include <iostream>
#include <thread>

// Icons.
char playerIcon{'>'};
char enemyIcon{'X'};

// Bullet state.
bool bulletActive = false;
int bulletX{0};
int bulletY{0};
int bulletPrevX{0};
int bulletPrevY{0};
int bulletDX{0};
int bulletDY{0};

// Player state.
int prevPlayerX{50};
int prevPlayerY{7};
int playerDX{1};
int playerDY{0};
int playerX = maxX / 4;
int playerY = maxY / 2;
int playerScore = 0;

void movePlayer()
{
    prevPlayerX = playerX;
    prevPlayerY = playerY;

    if (inputState.up)
    {
        playerY--;
    }
    if (inputState.down)
    {
        playerY++;
    }
    if (inputState.left)
    {
        playerX--;
    }
    if (inputState.right && playerX < maxX / 4)
    {
        playerX++;
    }
    if (inputState.shoot)
    {
        fireBullet();
    }

    if (playerX < minX) playerX = minX;
    if (playerX > maxX) playerX = maxX;
    if (playerY < minY) playerY = minY;
    if (playerY > maxY) playerY = maxY;
}

void fireBullet()
{
    if (bulletActive)
    {
        return;
    }

    bulletActive = true;
    std::cout << "\a" << std::flush;

    const int bulletDistance = maxX - playerX;

    for (int i = 0; i <= bulletDistance; i++)
    {
        if ((playerX + playerDX * i) >= maxX || (playerY + playerDY * i) >= maxY)
        {
            bulletActive = false;
            return;
        }

        bulletPrevX = bulletX;
        bulletPrevY = bulletY;

        bulletX = playerX + playerDX * i;
        bulletY = playerY + playerDY * i;

        if (bulletPrevX != bulletX || bulletPrevY != bulletY)
        {
            if (bulletPrevX >= 1 && bulletPrevX <= 99 &&
                bulletPrevY >= 1 && bulletPrevY <= 13)
            {
                setCursorPos(bulletPrevX, bulletPrevY);
                std::cout << ' ';
            }
        }
        if (tryHitEnemyAt(bulletX, bulletY))
        {
            playerScore++;
            setCursorPos(bulletX, bulletY);
            std::cout << ' ';
            bulletActive = false;
            return;
        }
        render();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    bulletActive = false;
}

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
int playerHealth = 100;

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

    bulletPrevX = bulletX;
    bulletPrevY = bulletY;
    bulletX = playerX;
    bulletY = playerY;
}

void updateBullet()
{
    if (!bulletActive)
    {
        return;
    }

    bulletPrevX = bulletX;
    bulletPrevY = bulletY;

    int newBulletX = bulletX + playerDX;
    int newBulletY = bulletY + playerDY;

    if (newBulletX >= maxX || newBulletY >= maxY)
    {
        setCursorPos(bulletX, bulletY);
        std::cout << ' ';
        bulletActive = false;
        return;
    }

    if (tryHitEnemyAt(newBulletX, newBulletY))
    {
        setCursorPos(bulletX, bulletY);
        std::cout << ' ';
        playerScore++;
        bulletActive = false;
        return;
    }

    bulletX = newBulletX;
    bulletY = newBulletY;
}

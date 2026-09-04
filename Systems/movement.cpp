#include <iostream>
#include "movement.h"
#include <thread>
#include <chrono>
#include <string>
#include "render.h"

using namespace std;

void gotoXY(int x, int y);
void gotoXY(int x, int y, string text);

int playerX = 50;
int playerY = 7;
int prevPlayerX = 50;
int prevPlayerY = 7;
char playerIcon = '>';
char enemyIcon = 'X';

int playerDX = 1;
int playerDY = 0;

bool bulletActive = false;
int bulletX = 0;
int bulletY = 0;
int bulletPrevX = 0;
int bulletPrevY = 0;
int bulletDX = 0;
int bulletDY = 0;
int bulletSpeed = .25;

static const int minX = 1;
static const int maxX = 99;
static const int minY = 1;
static const int maxY = 13;

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
    if (inputState.right && playerX < maxX/4)
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
    if(bulletActive){
        return;
    }
    std::cout << "\a" << std::flush;
    bulletActive = true;
    int bulletDistance = 10;
    std::cout << "\a" << std::flush;
    for (int i = 0; i <= bulletDistance; i++)
    {
        if(((playerX + playerDX * i) >= maxX) || (playerY + playerDY * i) >= maxY){
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
        render();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    bulletActive = false;
}

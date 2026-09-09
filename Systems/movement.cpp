#include <iostream>
#include "movement.h"
#include <thread>
#include <chrono>
#include <string>
#include "render.h"

using namespace std;

void gotoXY(int x, int y), gotoXY(int X, int Y, string text);

//icons

char playerIcon{'>'}, enemyIcon{'X'};

// Here lies Bullet inilization

bool bulletActive = false;

int bulletX{0}, 
    bulletY{0}, 
    bulletPrevX{0}, 
    bulletPrevY{0}, 
    bulletDX{0}, 
    bulletDY{0},
    prevPlayerX{50},
    prevPlayerY{7},
    playerDX{1},
    playerDY{0};

double bulletSpeed{.25};
//---------------------------


//Player spawn
int playerX = maxX/4;
int playerY = maxY/2;
int prevPlayerx = playerX;
int prevPlayery = playerY;
//--------------------------

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

void startEnemies(){
    int enemyMaxDistance = 20;
    for (int i= 0; i<= enemyMaxDistance; i++)
    {

    }
    
}

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


bool 

using namespace std;

const int length = 100;
const int width = 100;

static std::random_device rd;
static std::mt19937 engine(rd());


//number of enemies to be spawned
int targetEnemyCount = 4;
//number of enemies currently spawned
int enemyCounter = 0;
bool allEnemiesSpawned = false;
// false if enemy corrispoding to that element is dead
bool enemyActivity[enemyCount] = {false, false, false false};
// all enemy positions are stored here
// [0][] = x, [1][] = y
int enemyXY[2][enemyCount];

int randomYvalue(){
    return std::uniform_int_distribution<int>(2, 13)(engine);
}


//If all enemies are active, this will move them forward once.
//to be called once per frame
//updates until enemies reach their max point
void updateEnemeis(std::vector<int>& YPositionArray, int enemyCount, int enemyX)
{
    //fix line below
    if(allEnemiesSpawned)
    {
         for(int i = 0; i < targetEnemyCount; i++)
         {
             int prevEnemyX = enemyX;
             enemyX--;
            //remove old enemy icon
            setCursorPos(prevEnemyX, YPositionArray[i]);
            std::cout << ' ';
            //create new enemy
            setCursorPos(enemyX, YPositionArray[i]);
            std::cout << enemyIcon;
         }
         movePlayer();
         std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

//Once at the start of a round, enemies will be spawned at a random y pos
//xy of enemies are stored as well as their status.
//called once then will pass every frame.
void initiateEnemySpawn(int numberOfEnemies)
{
    if(!allEnemiesSpawned)
    {
        int enemyY = randomYvalue();
        //Enemy Printing
        setCursorPos(maxX, enemyY);
        std::cout << enemyIcon;
        //add enemyY to Y value array
        enemyXY[1][enemyCounter] = enemyY;
        enemyXY[0][enemyCounter] = maxX; // all enemies start at maxX
        //set enemy to active
        enemyActivity[enemyCounter] = true;
        enemyCounter++;
        if(enemyCounter = targetEnemyCount)
        {
            allEnemiesSpawned = true;
        }
    }
}


// Sets cursor position for drawing elements
void setCursorPos(int x, int y){
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = {(short)x, (short)y};
    SetConsoleCursorPosition(hConsole, pos);
#else
    std::cout << "\033[" << (y + 1) << ";" << (x + 1) << "H" << std::flush;
#endif
}

void setColor(int colorCode){
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, colorCode);
#else
    if (colorCode == 11) {
        std::cout << "\033[36m";
    } else {
        std::cout << "\033[0m";
    }
#endif
}

void renderBorder(int width, int height)
{
    // 11 = cyan
    setColor(11);

    // Draw top/bottom borders
    for (int x = 1; x < width; ++x){
        //top
        setCursorPos(x, 0);
        std::cout << '_';
        //bottom
        setCursorPos(x, height - 1);
        std::cout << '_';
    }

    // Draw left/right borders
    for (int y = 0; y < height - 1; ++y){
        //left
        setCursorPos(0, y + 1);
        std::cout << '|';
        //right
        setCursorPos(width, y + 1);
        std::cout << '|';
    }
    setColor(7);
}

void render(){
    if (prevPlayerX != playerX || prevPlayerY != playerY) {
        setCursorPos(prevPlayerX, prevPlayerY);
        std::cout << ' ';
    }

    if (bulletPrevX != bulletX || bulletPrevY != bulletY) {
        if (bulletPrevX >= 1 && bulletPrevX <= 99 && bulletPrevY >= 1 && bulletPrevY <= 13) {
            setCursorPos(bulletPrevX, bulletPrevY);
            std::cout << ' ';
        }
    }

    if (bulletActive) {
        setCursorPos(bulletX, bulletY);
        std::cout << '*';
    }

    setCursorPos(playerX, playerY);
    std::cout << playerIcon;
    setCursorPos(0, 16);

    movePlayer();
    renderEnemy();


}

//tofix
void renderEnemy(){
    startSpawn(4);
}

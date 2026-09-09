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

using namespace std;

const int length = 100;
const int width = 100;

static std::random_device rd;
static std::mt19937 engine(rd());

int randomYvalue(){
    return std::uniform_int_distribution<int>(2, 13)(engine);
}


void startEnemyMovement(std::vector<int>& YPositionArray, int enemyCount, int enemyX)
{
    while(enemyX > maxX/2)
    {
         for(int i = 0; i < enemyCount; i++)
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

// this function will print an enemy(X) 20% of the time.
// and will run continually until there are numberOfEnemies enemies
// then starts enemyMovement with startEnemyMovement
void startSpawn(int numberOfEnemies)
{

    int enemyCounter{0}, enemyYPosition{0};
    std::vector<int> enemyYvalues(numberOfEnemies);

    while(enemyCounter < numberOfEnemies)
    {
        int currentRandom = std::uniform_int_distribution<int>(0, 100)(engine);
        if(currentRandom <= 20)
        {
            int enemyY = randomYvalue();
            //Enemy Printing
            setCursorPos(maxX, enemyY);
            std::cout << enemyIcon;
            //add enemyY to Y value array
            enemyYvalues[enemyCounter] = enemyY;
            enemyCounter++;
        }
    }

    startEnemyMovement(enemyYvalues, numberOfEnemies, maxX);

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

void renderEnemy(){
    startSpawn(4);
}

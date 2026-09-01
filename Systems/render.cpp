#include "render.h"
#include "movement.h"
#include <iostream>
#include <windows.h>
using namespace std;

const int length = 100;
const int width = 100;

// Sets cursor position for drawing elements
void setCursorPos(int x, int y){
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = {(short)x, (short)y};
    SetConsoleCursorPosition(hConsole, pos);
}

void setColor(int colorCode){
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, colorCode);
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
}

#include "setup.h"
#include "render.h"
#include <iostream>
#include <windows.h>

bool gameOver = false;

static void hideCursor()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

bool setup()
{
    system("cls");
    hideCursor();
    gameOver = false;
    renderBorder(100, 15);
    return true;
}

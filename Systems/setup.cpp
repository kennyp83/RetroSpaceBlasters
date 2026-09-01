#include "setup.h"
#include "render.h"
#include "input.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

bool gameOver = false;

static void hideCursor()
{
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
#else
    std::cout << "\033[?25l" << std::flush;
#endif
}

bool setup()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
    enableRawInput();
#endif
    hideCursor();
    gameOver = false;
    renderBorder(100, 15);
    return true;
}

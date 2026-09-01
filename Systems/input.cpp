#include "input.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>

static char getCharNonBlocking() {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    int oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    char ch = 0;
    if (read(STDIN_FILENO, &ch, 1) <= 0) {
        ch = 0;
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    return ch;
}
#endif

InputState inputState;

static bool lastUp = false;
static bool lastDown = false;
static bool lastLeft = false;
static bool lastRight = false;
static bool lastShoot = false;

void input() {
#ifdef _WIN32
    bool currentUp = (GetAsyncKeyState('W') & 0x8000) || (GetAsyncKeyState(VK_UP) & 0x8000);
    bool currentDown = (GetAsyncKeyState('S') & 0x8000) || (GetAsyncKeyState(VK_DOWN) & 0x8000);
    bool currentLeft = (GetAsyncKeyState('A') & 0x8000) || (GetAsyncKeyState(VK_LEFT) & 0x8000);
    bool currentRight = (GetAsyncKeyState('D') & 0x8000) || (GetAsyncKeyState(VK_RIGHT) & 0x8000);
    bool currentShoot = (GetAsyncKeyState(VK_SPACE) & 0x8000);
#else
    char c = getCharNonBlocking();
    bool currentUp = (c == 'w' || c == 'W');
    bool currentDown = (c == 's' || c == 'S');
    bool currentLeft = (c == 'a' || c == 'A');
    bool currentRight = (c == 'd' || c == 'D');
    bool currentShoot = (c == ' ');

    if (c == '\033') {
        char seq[2];
        if (read(STDIN_FILENO, &seq[0], 1) > 0 && read(STDIN_FILENO, &seq[1], 1) > 0) {
            if (seq[0] == '[') {
                switch (seq[1]) {
                    case 'A': currentUp = true; break;
                    case 'B': currentDown = true; break;
                    case 'D': currentLeft = true; break;
                    case 'C': currentRight = true; break;
                }
            }
        }
    }
#endif

    inputState.up = currentUp && !lastUp;
    inputState.down = currentDown && !lastDown;
    inputState.left = currentLeft && !lastLeft;
    inputState.right = currentRight && !lastRight;
    inputState.shoot = currentShoot && !lastShoot;

    lastUp = currentUp;
    lastDown = currentDown;
    lastLeft = currentLeft;
    lastRight = currentRight;
    lastShoot = currentShoot;
}

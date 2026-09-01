#include "input.h"
#include <windows.h>

InputState inputState;

static bool lastUp = false;
static bool lastDown = false;
static bool lastLeft = false;
static bool lastRight = false;
static bool lastShoot = false;

void input() {
    bool currentUp = (GetAsyncKeyState('W') & 0x8000) || (GetAsyncKeyState(VK_UP) & 0x8000);
    bool currentDown = (GetAsyncKeyState('S') & 0x8000) || (GetAsyncKeyState(VK_DOWN) & 0x8000);
    bool currentLeft = (GetAsyncKeyState('A') & 0x8000) || (GetAsyncKeyState(VK_LEFT) & 0x8000);
    bool currentRight = (GetAsyncKeyState('D') & 0x8000) || (GetAsyncKeyState(VK_RIGHT) & 0x8000);
    bool currentShoot = (GetAsyncKeyState(VK_SPACE) & 0x8000);

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

#include "input.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <cstdlib>

static termios originalTerminalSettings;
static int originalInputFlags;
static bool rawInputEnabled = false;

void disableRawInput()
{
	if(!rawInputEnabled){
		return;
	}

	tcsetattr(STDIN_FILENO, TCSANOW, &originalTerminalSettings);
    fcntl(STDIN_FILENO, F_SETFL, originalInputFlags);
	rawInputEnabled = false;

}
void enableRawInput()
{
	if(rawInputEnabled){
		return;
	}

	tcgetattr(STDIN_FILENO, &originalTerminalSettings);

	termios rawSettings = originalTerminalSettings;
	rawSettings.c_lflag &= ~(ICANON | ECHO);
	rawSettings.c_cc[VMIN] = 0;
	rawSettings.c_cc[VTIME] = 0;

	tcsetattr(STDIN_FILENO, TCSANOW, &rawSettings);

	originalInputFlags = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, originalInputFlags | O_NONBLOCK);

	rawInputEnabled = true;
	std::atexit(disableRawInput);

	
}

static char getCharNonBlocking() {
	char character = 0;
	if (read(STDIN_FILENO, &character, 1) != 1){
		return 0;
 }
	return character;
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

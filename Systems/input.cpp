#include "input.h"
#include "setup.h"

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
    if (!rawInputEnabled)
    {
        return;
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &originalTerminalSettings);
    fcntl(STDIN_FILENO, F_SETFL, originalInputFlags);
    rawInputEnabled = false;
}

void enableRawInput()
{
    if (rawInputEnabled)
    {
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

struct RawInputBatch
{
    bool sawUp = false;
    bool sawDown = false;
    bool sawLeft = false;
    bool sawRight = false;
    bool sawShoot = false;
    bool sawEscape = false;
};

// Drains every byte queued this frame so a burst of keystrokes can't be spread across frames.
static RawInputBatch drainRawInput()
{
    RawInputBatch batch;
    char c;
    while (read(STDIN_FILENO, &c, 1) == 1)
    {
        switch (c)
        {
            case 'w': case 'W': batch.sawUp = true; break;
            case 's': case 'S': batch.sawDown = true; break;
            case 'a': case 'A': batch.sawLeft = true; break;
            case 'd': case 'D': batch.sawRight = true; break;
            case ' ': batch.sawShoot = true; break;
            case '\033':
            {
                char seq[2];
                bool gotSeq = read(STDIN_FILENO, &seq[0], 1) > 0 && read(STDIN_FILENO, &seq[1], 1) > 0;
                if (gotSeq && seq[0] == '[')
                {
                    switch (seq[1])
                    {
                        case 'A': batch.sawUp = true; break;
                        case 'B': batch.sawDown = true; break;
                        case 'D': batch.sawLeft = true; break;
                        case 'C': batch.sawRight = true; break;
                    }
                }
                else if (!gotSeq)
                {
                    batch.sawEscape = true;
                }
                break;
            }
        }
    }
    return batch;
}
#endif

InputState inputState;

static bool lastUp = false;
static bool lastDown = false;
static bool lastLeft = false;
static bool lastRight = false;

void input() {
#ifdef _WIN32
    bool currentUp = (GetAsyncKeyState('W') & 0x8000) || (GetAsyncKeyState(VK_UP) & 0x8000);
    bool currentDown = (GetAsyncKeyState('S') & 0x8000) || (GetAsyncKeyState(VK_DOWN) & 0x8000);
    bool currentLeft = (GetAsyncKeyState('A') & 0x8000) || (GetAsyncKeyState(VK_LEFT) & 0x8000);
    bool currentRight = (GetAsyncKeyState('D') & 0x8000) || (GetAsyncKeyState(VK_RIGHT) & 0x8000);
    bool currentShoot = (GetAsyncKeyState(VK_SPACE) & 0x8000);
    bool currentEscape = (GetAsyncKeyState(VK_ESCAPE) & 0x8000);
    if (currentEscape)
    {
        gameOver = true;
    }
#else
    RawInputBatch batch = drainRawInput();
    bool currentUp = batch.sawUp;
    bool currentDown = batch.sawDown;
    bool currentLeft = batch.sawLeft;
    bool currentRight = batch.sawRight;
    bool currentShoot = batch.sawShoot;
    if (batch.sawEscape)
    {
        gameOver = true;
    }
#endif

    inputState.up = currentUp && !lastUp;
    inputState.down = currentDown && !lastDown;
    inputState.left = currentLeft && !lastLeft;
    inputState.right = currentRight && !lastRight;
    inputState.shoot = currentShoot; // debounced by bulletActive in fireBullet(), not edge-detection

    lastUp = currentUp;
    lastDown = currentDown;
    lastLeft = currentLeft;
    lastRight = currentRight;
}

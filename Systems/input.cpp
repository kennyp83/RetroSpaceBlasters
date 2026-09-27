#include "input.h"
#include "setup.h"

#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#else
#include <cerrno>
#include <cstdlib>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#endif

namespace {

struct PolledKeys {
    bool up;
    bool down;
    bool left;
    bool right;
    bool shoot;
    bool quit;
};

#ifdef _WIN32

PolledKeys pollKeys()
{
    PolledKeys keys = {false, false, false, false, false, false};
    keys.up = (GetAsyncKeyState('W') & 0x8000) || (GetAsyncKeyState(VK_UP) & 0x8000);
    keys.down = (GetAsyncKeyState('S') & 0x8000) || (GetAsyncKeyState(VK_DOWN) & 0x8000);
    keys.left = (GetAsyncKeyState('A') & 0x8000) || (GetAsyncKeyState(VK_LEFT) & 0x8000);
    keys.right = (GetAsyncKeyState('D') & 0x8000) || (GetAsyncKeyState(VK_RIGHT) & 0x8000);
    keys.shoot = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;
    keys.quit = (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;
    return keys;
}

#else

termios originalTerminalSettings;
int originalInputFlags = 0;
bool rawInputEnabled = false;

enum class EscapeParse : std::uint8_t {
    None,
    SawEsc,
    SawFinal
};

EscapeParse escapeParse = EscapeParse::None;
int escapeWaitFrames = 0;

// A lone Esc has no follower. Wait one extra poll so a split arrow sequence is not a quit.
constexpr int kEscapeConfirmFrames = 2;

int readByte(char& out)
{
    for (;;) {
        const ssize_t readCount = ::read(STDIN_FILENO, &out, 1);
        if (readCount == 1) {
            return 1;
        }
        if (readCount < 0 && errno == EINTR) {
            continue;
        }
        return 0;
    }
}

void acceptByte(char c, PolledKeys& keys)
{
    if (escapeParse == EscapeParse::SawEsc) {
        if (c == '[' || c == 'O') {
            escapeParse = EscapeParse::SawFinal;
            escapeWaitFrames = 0;
            return;
        }
        keys.quit = true;
        escapeParse = EscapeParse::None;
        escapeWaitFrames = 0;
    } else if (escapeParse == EscapeParse::SawFinal) {
        escapeParse = EscapeParse::None;
        escapeWaitFrames = 0;
        switch (c) {
        case 'A':
            keys.up = true;
            return;
        case 'B':
            keys.down = true;
            return;
        case 'C':
            keys.right = true;
            return;
        case 'D':
            keys.left = true;
            return;
        default:
            return;
        }
    }

    switch (c) {
    case 'w':
    case 'W':
        keys.up = true;
        break;
    case 's':
    case 'S':
        keys.down = true;
        break;
    case 'a':
    case 'A':
        keys.left = true;
        break;
    case 'd':
    case 'D':
        keys.right = true;
        break;
    case ' ':
        keys.shoot = true;
        break;
    case '\033':
        escapeParse = EscapeParse::SawEsc;
        escapeWaitFrames = 0;
        break;
    default:
        break;
    }
}

void expireEscape(PolledKeys& keys)
{
    if (escapeParse == EscapeParse::None) {
        escapeWaitFrames = 0;
        return;
    }

    ++escapeWaitFrames;
    if (escapeWaitFrames < kEscapeConfirmFrames) {
        return;
    }

    if (escapeParse == EscapeParse::SawEsc) {
        keys.quit = true;
    }
    escapeParse = EscapeParse::None;
    escapeWaitFrames = 0;
}

PolledKeys pollKeys()
{
    PolledKeys keys = {false, false, false, false, false, false};
    char c = 0;
    while (readByte(c) == 1) {
        acceptByte(c, keys);
    }
    expireEscape(keys);
    return keys;
}

#endif

}

InputState inputState;

namespace {

bool lastUp = false;
bool lastDown = false;
bool lastLeft = false;
bool lastRight = false;

}

#ifdef _WIN32

void disableRawInput()
{
}

void enableRawInput()
{
}

#else

void disableRawInput()
{
    if (!rawInputEnabled) {
        return;
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &originalTerminalSettings);
    fcntl(STDIN_FILENO, F_SETFL, originalInputFlags);
    rawInputEnabled = false;
    escapeParse = EscapeParse::None;
    escapeWaitFrames = 0;
}

void enableRawInput()
{
    if (rawInputEnabled) {
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
    escapeParse = EscapeParse::None;
    escapeWaitFrames = 0;
    std::atexit(disableRawInput);
}

#endif

void input()
{
    const PolledKeys keys = pollKeys();

    inputState.up = keys.up && !lastUp;
    inputState.down = keys.down && !lastDown;
    inputState.left = keys.left && !lastLeft;
    inputState.right = keys.right && !lastRight;
    inputState.shoot = keys.shoot;
    inputState.quit = keys.quit;
    if (keys.quit) {
        gameOver = true;
    }

    lastUp = keys.up;
    lastDown = keys.down;
    lastLeft = keys.left;
    lastRight = keys.right;
}

void applyInput(EntityManager& world, Entity player)
{
    if (inputState.quit) {
        world.match.gameOver = true;
    }
    if (!world.alive(player) || !world.has<Intent>(player)) {
        return;
    }

    Intent& intent = world.get<Intent>(player);
    intent.dx = (inputState.right ? 1 : 0) - (inputState.left ? 1 : 0);
    intent.dy = (inputState.down ? 1 : 0) - (inputState.up ? 1 : 0);
    intent.shoot = inputState.shoot;
}

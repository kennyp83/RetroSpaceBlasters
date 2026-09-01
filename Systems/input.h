#pragma once

struct InputState {
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
    bool shoot = false;
};

extern InputState inputState;

void enableRawInput();
void disableRawInput();
void input();

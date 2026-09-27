#pragma once

#include "../ECS/entityManager.h"

struct InputState {
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
    bool shoot = false;
    bool quit = false;
};

extern InputState inputState;

void enableRawInput();
void disableRawInput();
void input();

// Publishes the InputState from this frame's input() call. Screen Y grows downward.
void applyInput(EntityManager& world, Entity player);

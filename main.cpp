#include "ECS/entityManager.h"
#include "Systems/draw.h"
#include "Systems/input.h"
#include "Systems/render.h"
#include "Systems/setup.h"
#include "Systems/simulation.h"

#include <chrono>
#include <iostream>
#include <thread>

namespace {
constexpr int frameDelayMs = 16;
}

int main()
{
    setup();
    EntityManager world;
    const Entity player = beginMatch(world);
    drawFrame(world);

    while (!gameOver && !world.match.gameOver) {
        input();
        applyInput(world, player);
        if (gameOver || world.match.gameOver) {
            break;
        }
        simulate(world);
        drawFrame(world);
        std::this_thread::sleep_for(std::chrono::milliseconds(frameDelayMs));
    }

    drawFrame(world);
    if (world.match.gameOver && !inputState.quit) {
        setCursorPos(0, kHudRow + 1);
        std::cout << "Game over";
    }
    finishDisplay();
    return 0;
}

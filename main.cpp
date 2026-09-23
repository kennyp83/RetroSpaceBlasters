#include "Systems/render.h"
#include "Systems/setup.h"
#include "Systems/input.h"
#include "Systems/movement.h"
#include <chrono>
#include <thread>

extern bool gameOver;
bool setup();

namespace
{
    constexpr int frameDelayMs = 16; // ~60 FPS.
}

int main()
{
    setup();
    while (!gameOver)
    {
        input();
        render();
        std::this_thread::sleep_for(std::chrono::milliseconds(frameDelayMs));
    }
}

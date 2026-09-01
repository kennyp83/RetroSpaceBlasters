#include "Systems/render.h"
#include "Systems/setup.h"
#include "Systems/input.h"
#include "Systems/movement.h"
#include <chrono>
#include <thread>

extern bool gameOver;
bool setup();

int main(){
    setup();
    while (!gameOver)
    {
        input();
        movePlayer();
        render();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

#include "Systems/render.h"
#include "Systems/setup.h"
#include "Systems/input.h"
#include "Systems/movement.h"
#include <windows.h>
extern bool gameOver;
bool setup();

int main(){
    setup();
    while (!gameOver)
    {
        input();
        movePlayer();
        render();
    }
}

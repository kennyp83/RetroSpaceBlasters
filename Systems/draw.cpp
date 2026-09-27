#include "draw.h"
#include "input.h"
#include "render.h"

#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

namespace {

char shown[kMaxY + 1][kMaxX + 1];
bool shownReady = false;

void clearShown()
{
    for (int y = kMinY; y <= kMaxY; ++y) {
        for (int x = kMinX; x <= kMaxX; ++x) {
            shown[y][x] = ' ';
        }
    }
    shownReady = true;
}

void stamp(char field[kMaxY + 1][kMaxX + 1], const EntityManager& world, std::uint32_t tag)
{
    for (std::uint16_t index = 0; index < kMaxEntities; ++index) {
        const Entity entity = world.makeEntity(index);
        if (!world.tagged(entity, tag) || !world.has<Position>(entity) || !world.has<Glyph>(entity)) {
            continue;
        }

        const Position pos = world.get<Position>(entity);
        if (pos.x < kMinX || pos.x > kMaxX || pos.y < kMinY || pos.y > kMaxY) {
            continue;
        }
        field[pos.y][pos.x] = world.get<Glyph>(entity).icon;
    }
}

int playerHealth(const EntityManager& world)
{
    for (std::uint16_t index = 0; index < kMaxEntities; ++index) {
        const Entity entity = world.makeEntity(index);
        if (world.tagged(entity, kTagPlayer) && world.has<Health>(entity)) {
            return world.get<Health>(entity).current;
        }
    }
    return 0;
}

}

void drawFrame(const EntityManager& world)
{
    if (!shownReady) {
        clearShown();
    }

    char next[kMaxY + 1][kMaxX + 1];
    for (int y = kMinY; y <= kMaxY; ++y) {
        for (int x = kMinX; x <= kMaxX; ++x) {
            next[y][x] = ' ';
        }
    }

    stamp(next, world, kTagEnemy);
    stamp(next, world, kTagBullet);
    stamp(next, world, kTagPlayer);

    for (int y = kMinY; y <= kMaxY; ++y) {
        for (int x = kMinX; x <= kMaxX; ++x) {
            if (shown[y][x] == next[y][x]) {
                continue;
            }
            setCursorPos(x, y);
            std::cout << next[y][x];
            shown[y][x] = next[y][x];
        }
    }

    setCursorPos(0, kHudRow);
    std::cout << "Enemies through: " << world.match.enemiesThrough
              << " Score: " << world.match.score
              << " Health: " << playerHealth(world)
              << " Wave: " << world.match.wave
              << "    " << std::flush;
}

void finishDisplay()
{
    setCursorPos(0, kHudRow + 1);
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    if (GetConsoleCursorInfo(hConsole, &cursorInfo)) {
        cursorInfo.bVisible = TRUE;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
    }
#else
    std::cout << "\033[?25h";
#endif
    std::cout << '\n' << std::flush;
    disableRawInput();
}

#include "simulation.h"

#include <cstdint>
#include <iostream>

namespace {

std::uint32_t yRng = 0xC0FFEEu;

void resetSpawnRng()
{
    yRng = 0xC0FFEEu;
}

int nextSpawnY()
{
    yRng = yRng * 1664525u + 1013904223u;
    return 2 + static_cast<int>(yRng % 12u);
}

Entity findPlayer(const EntityManager& world)
{
    for (std::uint16_t index = 0; index < kMaxEntities; ++index) {
        const Entity entity = world.makeEntity(index);
        if (world.tagged(entity, kTagPlayer)) {
            return entity;
        }
    }
    return kNullEntity;
}

bool bulletAlive(const EntityManager& world)
{
    for (std::uint16_t index = 0; index < kMaxEntities; ++index) {
        if (world.tagged(world.makeEntity(index), kTagBullet)) {
            return true;
        }
    }
    return false;
}

int countEnemies(const EntityManager& world)
{
    int count = 0;
    for (std::uint16_t index = 0; index < kMaxEntities; ++index) {
        if (world.tagged(world.makeEntity(index), kTagEnemy)) {
            ++count;
        }
    }
    return count;
}

void erasePrevious(EntityManager& world, Entity entity)
{
    const Position& pos = world.get<Position>(entity);
    PreviousPosition& previous = world.get<PreviousPosition>(entity);
    previous.x = pos.x;
    previous.y = pos.y;
}

void applyPlayer(EntityManager& world, Entity player)
{
    if (!world.has<Position>(player) || !world.has<PreviousPosition>(player)
        || !world.has<Intent>(player) || !world.has<MovementBounds>(player)) {
        return;
    }

    Position& pos = world.get<Position>(player);
    const Intent intent = world.get<Intent>(player);
    const MovementBounds bounds = world.get<MovementBounds>(player);
    erasePrevious(world, player);

    int x = pos.x;
    int y = pos.y + intent.dy;
    if (!(intent.dx > 0 && pos.x >= bounds.maxX)) {
        x += intent.dx;
    }
    if (x < bounds.minX) {
        x = bounds.minX;
    }
    if (x > bounds.maxX) {
        x = bounds.maxX;
    }
    if (y < bounds.minY) {
        y = bounds.minY;
    }
    if (y > bounds.maxY) {
        y = bounds.maxY;
    }
    pos.x = x;
    pos.y = y;
}

void spawnBullet(EntityManager& world, int x, int y)
{
    const Entity bullet = world.create();
    if (!world.alive(bullet)) {
        return;
    }

    world.tag(bullet, kTagBullet);
    world.add(bullet, Position{x, y});
    world.add(bullet, PreviousPosition{x, y});
    world.add(bullet, Velocity{1, 0});
    world.add(bullet, Glyph{kBulletIcon, kColorText});
    world.add(bullet, Lifetime{0, kBulletMaxFrames});
    world.add(bullet, ScoreOnHit{kHitScore});
    std::cout << '\a' << std::flush;
}

void tryFire(EntityManager& world, Entity player)
{
    if (!world.has<Intent>(player) || !world.has<Position>(player)) {
        return;
    }
    if (!world.get<Intent>(player).shoot || bulletAlive(world)) {
        return;
    }

    const Position pos = world.get<Position>(player);
    spawnBullet(world, pos.x, pos.y);
}

void spawnEnemy(EntityManager& world, int x, int y)
{
    const Entity enemy = world.create();
    if (!world.alive(enemy)) {
        return;
    }

    world.tag(enemy, kTagEnemy);
    world.add(enemy, Position{x, y});
    world.add(enemy, PreviousPosition{x, y});
    world.add(enemy, Velocity{-1, 0});
    world.add(enemy, Glyph{kEnemyIcon, kColorText});
    world.add(enemy, Damage{kEnemyLeakDamage});
}

void spawnEnemyIfNeeded(EntityManager& world)
{
    MatchState& match = world.match;
    if (match.allEnemiesSpawned || match.enemiesSpawned >= match.targetEnemyCount) {
        return;
    }

    spawnEnemy(world, kMaxX, nextSpawnY());
    ++match.enemiesSpawned;
    if (match.enemiesSpawned >= match.targetEnemyCount) {
        match.allEnemiesSpawned = true;
    }
}

void damagePlayer(EntityManager& world, Entity player, int amount)
{
    if (!world.alive(player) || !world.has<Health>(player)) {
        return;
    }

    Health& health = world.get<Health>(player);
    health.current -= amount;
    if (health.current <= 0) {
        health.current = 0;
        world.match.gameOver = true;
    }
}

void moveBullets(EntityManager& world)
{
    for (std::uint16_t index = 0; index < kMaxEntities; ++index) {
        const Entity bullet = world.makeEntity(index);
        if (!world.tagged(bullet, kTagBullet) || !world.has<Position>(bullet)
            || !world.has<PreviousPosition>(bullet) || !world.has<Velocity>(bullet)) {
            continue;
        }

        if (world.has<Lifetime>(bullet)) {
            Lifetime& life = world.get<Lifetime>(bullet);
            ++life.framesAlive;
            if (life.framesAlive > life.maxFrames) {
                world.destroy(bullet);
                continue;
            }
        }

        erasePrevious(world, bullet);
        Position& pos = world.get<Position>(bullet);
        const Velocity velocity = world.get<Velocity>(bullet);
        pos.x += velocity.dx;
        pos.y += velocity.dy;
    }
}

void moveEnemies(EntityManager& world, Entity player)
{
    MatchState& match = world.match;
    if (!match.allEnemiesSpawned) {
        return;
    }

    ++match.enemyMoveTimer;
    if (match.enemyMoveTimer < match.enemyMoveInterval) {
        for (std::uint16_t index = 0; index < kMaxEntities; ++index) {
            const Entity enemy = world.makeEntity(index);
            if (world.tagged(enemy, kTagEnemy) && world.has<Position>(enemy)
                && world.has<PreviousPosition>(enemy)) {
                erasePrevious(world, enemy);
            }
        }
        return;
    }
    match.enemyMoveTimer = 0;

    for (std::uint16_t index = 0; index < kMaxEntities; ++index) {
        const Entity enemy = world.makeEntity(index);
        if (!world.tagged(enemy, kTagEnemy) || !world.has<Position>(enemy)
            || !world.has<PreviousPosition>(enemy)) {
            continue;
        }

        const Velocity velocity = world.has<Velocity>(enemy)
            ? world.get<Velocity>(enemy)
            : Velocity{-1, 0};
        Position& pos = world.get<Position>(enemy);
        erasePrevious(world, enemy);
        const int nextX = pos.x + velocity.dx;

        if (nextX <= kPlayerMaxX) {
            const int amount = world.has<Damage>(enemy)
                ? world.get<Damage>(enemy).amount
                : kEnemyLeakDamage;
            ++match.enemiesThrough;
            damagePlayer(world, player, amount);
            pos.x = kMaxX;
            pos.y = nextSpawnY();
            continue;
        }

        pos.x = nextX;
    }
}

bool sameCell(Position lhs, Position rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

// Head-on pass: each entity lands on the other's previous cell.
bool crossed(Position fromA, Position toA, Position fromB, Position toB)
{
    return sameCell(toA, fromB) && sameCell(fromA, toB) && !sameCell(fromA, toA);
}

void resolveHits(EntityManager& world)
{
    bool hitBullet[kMaxEntities] = {false};
    bool hitEnemy[kMaxEntities] = {false};
    int score = 0;

    for (std::uint16_t bulletIndex = 0; bulletIndex < kMaxEntities; ++bulletIndex) {
        const Entity bullet = world.makeEntity(bulletIndex);
        if (!world.tagged(bullet, kTagBullet) || !world.has<Position>(bullet)
            || !world.has<PreviousPosition>(bullet)) {
            continue;
        }

        const Position bulletPos = world.get<Position>(bullet);
        const PreviousPosition bulletPrevious = world.get<PreviousPosition>(bullet);
        const Position bulletFrom = {bulletPrevious.x, bulletPrevious.y};

        for (std::uint16_t enemyIndex = 0; enemyIndex < kMaxEntities; ++enemyIndex) {
            const Entity enemy = world.makeEntity(enemyIndex);
            if (hitEnemy[enemyIndex] || !world.tagged(enemy, kTagEnemy)
                || !world.has<Position>(enemy) || !world.has<PreviousPosition>(enemy)) {
                continue;
            }

            const Position enemyPos = world.get<Position>(enemy);
            const PreviousPosition enemyPrevious = world.get<PreviousPosition>(enemy);
            const Position enemyFrom = {enemyPrevious.x, enemyPrevious.y};
            if (!sameCell(bulletPos, enemyPos) && !crossed(bulletFrom, bulletPos, enemyFrom, enemyPos)) {
                continue;
            }

            hitBullet[bulletIndex] = true;
            hitEnemy[enemyIndex] = true;
            score += world.has<ScoreOnHit>(bullet)
                ? world.get<ScoreOnHit>(bullet).points
                : kHitScore;
            break;
        }
    }

    world.match.score += score;
    for (std::uint16_t index = 0; index < kMaxEntities; ++index) {
        if (hitBullet[index] || hitEnemy[index]) {
            world.destroy(world.makeEntity(index));
        }
    }
}

void expireOffscreenBullets(EntityManager& world)
{
    for (std::uint16_t index = 0; index < kMaxEntities; ++index) {
        const Entity bullet = world.makeEntity(index);
        if (!world.tagged(bullet, kTagBullet) || !world.has<Position>(bullet)) {
            continue;
        }

        const Position pos = world.get<Position>(bullet);
        if (pos.x < kMinX || pos.x >= kMaxX || pos.y < kMinY || pos.y > kMaxY) {
            world.destroy(bullet);
        }
    }
}

void clearWaveIfNeeded(EntityManager& world)
{
    MatchState& match = world.match;
    if (!match.allEnemiesSpawned || countEnemies(world) > 0) {
        return;
    }

    if (match.enemyMoveInterval > match.minEnemyMoveInterval) {
        --match.enemyMoveInterval;
    }
    ++match.wave;
    match.enemiesSpawned = 0;
    match.allEnemiesSpawned = false;
    match.enemyMoveTimer = 0;
}

}

Entity beginMatch(EntityManager& world)
{
    world.reset();
    resetSpawnRng();

    const Entity player = world.create();
    world.tag(player, kTagPlayer);
    world.add(player, Position{kPlayerMaxX, kMaxY / 2});
    world.add(player, PreviousPosition{kPlayerMaxX, kMaxY / 2});
    world.add(player, Velocity{0, 0});
    world.add(player, Glyph{kPlayerIcon, kColorText});
    world.add(player, Health{kPlayerMaxHealth, kPlayerMaxHealth});
    world.add(player, MovementBounds{kMinX, kPlayerMaxX, kMinY, kMaxY});
    world.add(player, Intent{0, 0, false});
    return player;
}

void simulate(EntityManager& world)
{
    if (world.match.gameOver) {
        return;
    }

    const Entity player = findPlayer(world);
    if (world.alive(player)) {
        applyPlayer(world, player);
        tryFire(world, player);
    }

    spawnEnemyIfNeeded(world);
    moveBullets(world);
    moveEnemies(world, player);
    resolveHits(world);
    expireOffscreenBullets(world);
    clearWaveIfNeeded(world);
}

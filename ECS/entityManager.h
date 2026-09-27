#pragma once

#include "components.h"

#include <cassert>
#include <cstdint>
#include <type_traits>

inline constexpr std::uint16_t kMaxEntities = 32;

inline constexpr int kMinX = 1;
inline constexpr int kMaxX = 99;
inline constexpr int kMinY = 1;
inline constexpr int kMaxY = 13;
inline constexpr int kFieldWidth = 100;
inline constexpr int kFieldHeight = 15;
inline constexpr int kHudRow = 16;
inline constexpr int kPlayerMaxX = kMaxX / 4;
inline constexpr int kStartingEnemyInterval = 8;
inline constexpr int kMinimumEnemyInterval = 2;
inline constexpr int kEnemiesPerWave = 4;
inline constexpr int kPlayerMaxHealth = 100;
inline constexpr int kEnemyLeakDamage = 10;
inline constexpr int kBulletMaxFrames = 200;
inline constexpr int kHitScore = 1;
inline constexpr char kPlayerIcon = '>';
inline constexpr char kEnemyIcon = 'X';
inline constexpr char kBulletIcon = '*';
inline constexpr unsigned char kColorText = 7;
inline constexpr unsigned char kColorBorder = 11;

inline constexpr std::uint32_t kTagPlayer = std::uint32_t{1} << 16;
inline constexpr std::uint32_t kTagEnemy = std::uint32_t{1} << 17;
inline constexpr std::uint32_t kTagBullet = std::uint32_t{1} << 18;
inline constexpr std::uint32_t kTagMask = kTagPlayer | kTagEnemy | kTagBullet;

struct Entity {
    std::uint16_t index;
    std::uint16_t generation;
};

inline constexpr Entity kNullEntity = {0, 0};

inline bool operator==(Entity lhs, Entity rhs)
{
    return lhs.index == rhs.index && lhs.generation == rhs.generation;
}

inline bool operator!=(Entity lhs, Entity rhs)
{
    return !(lhs == rhs);
}

struct MatchState {
    int score;
    int enemiesThrough;
    int wave;
    int enemyMoveInterval;
    int minEnemyMoveInterval;
    int enemyMoveTimer;
    int enemiesSpawned;
    int targetEnemyCount;
    bool allEnemiesSpawned;
    bool gameOver;
};

template <typename T>
struct ComponentSpan {
    T* data;
    const std::uint16_t* entityIndices;
    std::uint16_t count;
};

template <typename T>
struct UnregisteredComponent {
    static constexpr bool value = false;
};

class EntityManager {
public:
    EntityManager();

    EntityManager(const EntityManager&) = delete;
    EntityManager& operator=(const EntityManager&) = delete;
    EntityManager(EntityManager&&) = delete;
    EntityManager& operator=(EntityManager&&) = delete;

    void reset();
    void resetMatch();

    [[nodiscard]] Entity create();
    void destroy(Entity entity);
    [[nodiscard]] bool alive(Entity entity) const;
    [[nodiscard]] std::uint16_t size() const;
    [[nodiscard]] static constexpr std::uint16_t capacity();
    [[nodiscard]] Entity makeEntity(std::uint16_t index) const;
    [[nodiscard]] std::uint32_t signature(Entity entity) const;

    void tag(Entity entity, std::uint32_t flags);
    void untag(Entity entity, std::uint32_t flags);
    [[nodiscard]] bool tagged(Entity entity, std::uint32_t flags) const;

    template <typename T>
    T& add(Entity entity, T value);

    // Dense indices are not stable across remove or destroy.
    template <typename T>
    void remove(Entity entity);

    template <typename T>
    [[nodiscard]] bool has(Entity entity) const;

    template <typename T>
    [[nodiscard]] T& get(Entity entity);

    template <typename T>
    [[nodiscard]] const T& get(Entity entity) const;

    template <typename T>
    [[nodiscard]] ComponentSpan<T> span();

    template <typename T>
    [[nodiscard]] ComponentSpan<const T> span() const;

    MatchState match;

private:
    // generation 0 is reserved so kNullEntity can never resolve to a live slot.
    struct EntitySlot {
        std::uint16_t generation;
        std::uint8_t alive;
        std::uint8_t reserved;
        std::uint32_t mask;
    };

    template <typename T>
    struct ComponentPool {
        T values[kMaxEntities];
        std::uint16_t entities[kMaxEntities];
        std::uint16_t sparse[kMaxEntities];
        std::uint16_t count;
    };

    template <typename T>
    [[nodiscard]] constexpr std::uint32_t componentBit() const;

    template <typename T>
    [[nodiscard]] ComponentPool<T>& poolFor();

    template <typename T>
    [[nodiscard]] const ComponentPool<T>& poolFor() const;

    template <typename T>
    void clearPool(ComponentPool<T>& pool);

    EntitySlot slots[kMaxEntities];
    std::uint16_t freeStack[kMaxEntities];
    std::uint16_t freeCount;
    std::uint16_t liveCount;

    ComponentPool<Position> positions;
    ComponentPool<PreviousPosition> previousPositions;
    ComponentPool<Velocity> velocities;
    ComponentPool<Glyph> glyphs;
    ComponentPool<Health> healths;
    ComponentPool<Lifetime> lifetimes;
    ComponentPool<Damage> damages;
    ComponentPool<ScoreOnHit> scoreOnHits;
    ComponentPool<MovementBounds> movementBounds;
    ComponentPool<Intent> intents;
};

inline EntityManager::EntityManager()
{
    reset();
}

inline void EntityManager::reset()
{
    freeCount = kMaxEntities;
    liveCount = 0;

    for (std::uint16_t i = 0; i < kMaxEntities; ++i) {
        slots[i].generation = 0;
        slots[i].alive = 0;
        slots[i].reserved = 0;
        slots[i].mask = 0;
        freeStack[i] = static_cast<std::uint16_t>(kMaxEntities - 1U - i);
    }

    clearPool(positions);
    clearPool(previousPositions);
    clearPool(velocities);
    clearPool(glyphs);
    clearPool(healths);
    clearPool(lifetimes);
    clearPool(damages);
    clearPool(scoreOnHits);
    clearPool(movementBounds);
    clearPool(intents);
    resetMatch();
}

inline void EntityManager::resetMatch()
{
    match.score = 0;
    match.enemiesThrough = 0;
    match.wave = 1;
    match.enemyMoveInterval = kStartingEnemyInterval;
    match.minEnemyMoveInterval = kMinimumEnemyInterval;
    match.enemyMoveTimer = 0;
    match.enemiesSpawned = 0;
    match.targetEnemyCount = kEnemiesPerWave;
    match.allEnemiesSpawned = false;
    match.gameOver = false;
}

inline Entity EntityManager::create()
{
    if (freeCount == 0) {
        return kNullEntity;
    }

    --freeCount;
    const std::uint16_t index = freeStack[freeCount];
    EntitySlot& slot = slots[index];
    if (slot.generation == 0) {
        slot.generation = 1;
    }
    slot.alive = 1;
    slot.mask = 0;
    ++liveCount;
    return Entity{index, slot.generation};
}

inline void EntityManager::destroy(Entity entity)
{
    if (!alive(entity)) {
        return;
    }

    remove<Position>(entity);
    remove<PreviousPosition>(entity);
    remove<Velocity>(entity);
    remove<Glyph>(entity);
    remove<Health>(entity);
    remove<Lifetime>(entity);
    remove<Damage>(entity);
    remove<ScoreOnHit>(entity);
    remove<MovementBounds>(entity);
    remove<Intent>(entity);

    EntitySlot& slot = slots[entity.index];
    slot.mask = 0;
    slot.alive = 0;

    std::uint16_t generation = static_cast<std::uint16_t>(slot.generation + 1U);
    if (generation == 0) {
        generation = 1;
    }
    slot.generation = generation;

    freeStack[freeCount] = entity.index;
    ++freeCount;
    --liveCount;
}

inline bool EntityManager::alive(Entity entity) const
{
    return entity.generation != 0
        && entity.index < kMaxEntities
        && slots[entity.index].alive != 0
        && slots[entity.index].generation == entity.generation;
}

inline std::uint16_t EntityManager::size() const
{
    return liveCount;
}

inline constexpr std::uint16_t EntityManager::capacity()
{
    return kMaxEntities;
}

inline Entity EntityManager::makeEntity(std::uint16_t index) const
{
    if (index >= kMaxEntities || slots[index].alive == 0) {
        return kNullEntity;
    }
    return Entity{index, slots[index].generation};
}

inline std::uint32_t EntityManager::signature(Entity entity) const
{
    if (!alive(entity)) {
        return 0;
    }
    return slots[entity.index].mask;
}

inline void EntityManager::tag(Entity entity, std::uint32_t flags)
{
    assert(alive(entity));
    slots[entity.index].mask |= (flags & kTagMask);
}

inline void EntityManager::untag(Entity entity, std::uint32_t flags)
{
    assert(alive(entity));
    slots[entity.index].mask &= ~(flags & kTagMask);
}

inline bool EntityManager::tagged(Entity entity, std::uint32_t flags) const
{
    if (!alive(entity)) {
        return false;
    }
    const std::uint32_t requested = flags & kTagMask;
    return (slots[entity.index].mask & requested) == requested;
}

template <typename T>
constexpr std::uint32_t EntityManager::componentBit() const
{
    if constexpr (std::is_same_v<T, Position>) {
        return std::uint32_t{1} << 0;
    } else if constexpr (std::is_same_v<T, PreviousPosition>) {
        return std::uint32_t{1} << 1;
    } else if constexpr (std::is_same_v<T, Velocity>) {
        return std::uint32_t{1} << 2;
    } else if constexpr (std::is_same_v<T, Glyph>) {
        return std::uint32_t{1} << 3;
    } else if constexpr (std::is_same_v<T, Health>) {
        return std::uint32_t{1} << 4;
    } else if constexpr (std::is_same_v<T, Lifetime>) {
        return std::uint32_t{1} << 5;
    } else if constexpr (std::is_same_v<T, Damage>) {
        return std::uint32_t{1} << 6;
    } else if constexpr (std::is_same_v<T, ScoreOnHit>) {
        return std::uint32_t{1} << 7;
    } else if constexpr (std::is_same_v<T, MovementBounds>) {
        return std::uint32_t{1} << 8;
    } else if constexpr (std::is_same_v<T, Intent>) {
        return std::uint32_t{1} << 9;
    } else {
        static_assert(UnregisteredComponent<T>::value, "Unregistered component type");
        return 0;
    }
}

template <typename T>
EntityManager::ComponentPool<T>& EntityManager::poolFor()
{
    if constexpr (std::is_same_v<T, Position>) {
        return positions;
    } else if constexpr (std::is_same_v<T, PreviousPosition>) {
        return previousPositions;
    } else if constexpr (std::is_same_v<T, Velocity>) {
        return velocities;
    } else if constexpr (std::is_same_v<T, Glyph>) {
        return glyphs;
    } else if constexpr (std::is_same_v<T, Health>) {
        return healths;
    } else if constexpr (std::is_same_v<T, Lifetime>) {
        return lifetimes;
    } else if constexpr (std::is_same_v<T, Damage>) {
        return damages;
    } else if constexpr (std::is_same_v<T, ScoreOnHit>) {
        return scoreOnHits;
    } else if constexpr (std::is_same_v<T, MovementBounds>) {
        return movementBounds;
    } else if constexpr (std::is_same_v<T, Intent>) {
        return intents;
    } else {
        static_assert(UnregisteredComponent<T>::value, "Unregistered component type");
        return *static_cast<ComponentPool<T>*>(nullptr);
    }
}

template <typename T>
const EntityManager::ComponentPool<T>& EntityManager::poolFor() const
{
    if constexpr (std::is_same_v<T, Position>) {
        return positions;
    } else if constexpr (std::is_same_v<T, PreviousPosition>) {
        return previousPositions;
    } else if constexpr (std::is_same_v<T, Velocity>) {
        return velocities;
    } else if constexpr (std::is_same_v<T, Glyph>) {
        return glyphs;
    } else if constexpr (std::is_same_v<T, Health>) {
        return healths;
    } else if constexpr (std::is_same_v<T, Lifetime>) {
        return lifetimes;
    } else if constexpr (std::is_same_v<T, Damage>) {
        return damages;
    } else if constexpr (std::is_same_v<T, ScoreOnHit>) {
        return scoreOnHits;
    } else if constexpr (std::is_same_v<T, MovementBounds>) {
        return movementBounds;
    } else if constexpr (std::is_same_v<T, Intent>) {
        return intents;
    } else {
        static_assert(UnregisteredComponent<T>::value, "Unregistered component type");
        return *static_cast<const ComponentPool<T>*>(nullptr);
    }
}

template <typename T>
void EntityManager::clearPool(ComponentPool<T>& pool)
{
    pool = ComponentPool<T>{};
}

template <typename T>
T& EntityManager::add(Entity entity, T value)
{
    assert(alive(entity));
    ComponentPool<T>& pool = poolFor<T>();
    const std::uint16_t existing = pool.sparse[entity.index];
    if (existing != 0) {
        T& stored = pool.values[existing - 1];
        stored = value;
        slots[entity.index].mask |= componentBit<T>();
        return stored;
    }

    assert(pool.count < kMaxEntities);
    const std::uint16_t dense = pool.count;
    pool.values[dense] = value;
    pool.entities[dense] = entity.index;
    pool.sparse[entity.index] = static_cast<std::uint16_t>(dense + 1U);
    ++pool.count;
    slots[entity.index].mask |= componentBit<T>();
    return pool.values[dense];
}

template <typename T>
void EntityManager::remove(Entity entity)
{
    if (!alive(entity)) {
        return;
    }

    ComponentPool<T>& pool = poolFor<T>();
    const std::uint16_t existing = pool.sparse[entity.index];
    if (existing == 0) {
        return;
    }

    const std::uint16_t dense = static_cast<std::uint16_t>(existing - 1U);
    const std::uint16_t last = static_cast<std::uint16_t>(pool.count - 1U);
    if (dense != last) {
        pool.values[dense] = pool.values[last];
        const std::uint16_t moved = pool.entities[last];
        pool.entities[dense] = moved;
        pool.sparse[moved] = existing;
    }

    pool.sparse[entity.index] = 0;
    pool.count = last;
    slots[entity.index].mask &= ~componentBit<T>();
}

template <typename T>
bool EntityManager::has(Entity entity) const
{
    if (!alive(entity)) {
        return false;
    }
    return (slots[entity.index].mask & componentBit<T>()) != 0;
}

template <typename T>
T& EntityManager::get(Entity entity)
{
    assert(has<T>(entity));
    ComponentPool<T>& pool = poolFor<T>();
    return pool.values[pool.sparse[entity.index] - 1U];
}

template <typename T>
const T& EntityManager::get(Entity entity) const
{
    assert(has<T>(entity));
    const ComponentPool<T>& pool = poolFor<T>();
    return pool.values[pool.sparse[entity.index] - 1U];
}

template <typename T>
ComponentSpan<T> EntityManager::span()
{
    ComponentPool<T>& pool = poolFor<T>();
    return ComponentSpan<T>{pool.values, pool.entities, pool.count};
}

template <typename T>
ComponentSpan<const T> EntityManager::span() const
{
    const ComponentPool<T>& pool = poolFor<T>();
    return ComponentSpan<const T>{pool.values, pool.entities, pool.count};
}

#pragma once

struct Position {
    int x;
    int y;
};

struct PreviousPosition {
    int x;
    int y;
};

struct Velocity {
    int dx;
    int dy;
};

struct Glyph {
    char icon;
    unsigned char color;
};

struct Health {
    int current;
    int maximum;
};

struct Lifetime {
    int framesAlive;
    int maxFrames;
};

struct Damage {
    int amount;
};

struct ScoreOnHit {
    int points;
};

struct MovementBounds {
    int minX;
    int maxX;
    int minY;
    int maxY;
};

struct Intent {
    int dx;
    int dy;
    bool shoot;
};
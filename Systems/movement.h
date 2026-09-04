#pragma once
#include "input.h"

extern int playerX;
extern int playerY;
extern int prevPlayerX;
extern int prevPlayerY;
extern char playerIcon;
extern char enemyIcon;

extern int playerDX;
extern int playerDY;

extern bool bulletActive;
extern int bulletX;
extern int bulletY;
extern int bulletPrevX;
extern int bulletPrevY;
extern int bulletDX;
extern int bulletDY;

void movePlayer();
void fireBullet();
void updateBullet();

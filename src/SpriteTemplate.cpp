/**
 * @file SpriteTemplate.cpp
 * @brief Sprite template implementation.
 */
// SpriteTemplate.cpp
#include "SpriteTemplate.h"
#include "LevelScene.h"
#include "Enemy.h"
#include "FlowerEnemy.h"

SpriteTemplate::SpriteTemplate(int type, bool winged)
    : type(type), winged(winged) {}

void SpriteTemplate::spawn(LevelScene* world, int x, int y, int dir) {
    if (isDead) return;
    
    if (type == Enemy::ENEMY_FLOWER) {
        sprite = new FlowerEnemy(world, x * 16 + 15, y * 16 + 24);
    } else {
        sprite = new Enemy(world, x * 16 + 8, y * 16 + 15, dir, type, winged);
    }
    sprite->spriteTemplate = this;
    world->addSprite(sprite);
}

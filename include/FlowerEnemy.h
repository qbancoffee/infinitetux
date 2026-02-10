/**
 * @file FlowerEnemy.h
 * @brief Piranha plant enemy.
 * @ingroup sprites
 * 
 * FlowerEnemy emerges from pipes and hides when
 * Mario approaches. Cannot be stomped.
 */
#pragma once
#include "Enemy.h"

class Shell;

/**
 * FlowerEnemy - Piranha Plant that emerges from pipes
 * 
 * Piranha plants move up and down from pipes. They are:
 * - Protected from fireballs AND shells when inside the pipe
 * - Vulnerable when extended above the pipe
 * - Cannot be stomped (uses ENEMY_SPIKY base type)
 */
class FlowerEnemy : public Enemy {
public:
    FlowerEnemy(LevelScene* world, int x, int y);
    
    void move() override;
    bool fireballCollideCheck(Fireball* fireball) override;
    bool shellCollideCheck(Shell* shell) override;
    
private:
    int tick = 0;
    int yStart;      ///< Y position of pipe top - plant is protected when y >= yStart - 8
    int jumpTime = 0;
};

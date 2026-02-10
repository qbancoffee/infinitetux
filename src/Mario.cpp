/**
 * @file Mario.cpp
 * @brief Player character implementation.
 */
#include "Mario.h"
#include "Art.h"
#include "Scene.h"
#include "LevelScene.h"
#include "Level.h"
#include "Enemy.h"
#include "Shell.h"
#include "BulletBill.h"
#include "Fireball.h"
#include "Sparkle.h"
#include <cmath>

// Static member initialization
bool Mario::large = false;
bool Mario::fire = false;
int Mario::coins = 0;
int Mario::lives = 3;
int Mario::score = 0;
std::string Mario::levelString = "none";
Mario* Mario::instance = nullptr;

void Mario::resetStatic() {
    large = false;
    fire = false;
    coins = 0;
    lives = 3;
    score = 0;
    levelString = "none";
}

void Mario::getCoin() {
    coins++;
    addScore(100);  // Coins give 100 points
    if (coins >= 100) {
        coins = 0;
        get1Up();
    }
}

void Mario::get1Up() {
    Art::playSound(SAMPLE_MARIO_1UP);
    lives++;
    if (lives > 99) lives = 99;
}

void Mario::addScore(int points) {
    score += points;
}

Mario::Mario(LevelScene* world) : world(world) {
    instance = this;
    keys = Scene::keys;
    x = 32;
    y = 0;
    facing = 1;
    layer = 1;  // Render in game layer
    setLarge(large, fire);
}

void Mario::blink(bool on) {
    large = on ? newLarge : lastLarge;
    fire = on ? newFire : lastFire;
    
    if (large) {
        sheet = &Art::mario;
        if (fire) sheet = &Art::fireMario;
        xPicO = 16;
        yPicO = 31;
        wPic = hPic = 32;
    } else {
        sheet = &Art::smallMario;
        xPicO = 8;
        yPicO = 15;
        wPic = hPic = 16;
    }
    
    calcPic();
}

void Mario::setLarge(bool large, bool fire) {
    if (fire) large = true;
    if (!large) fire = false;
    
    lastLarge = Mario::large;
    lastFire = Mario::fire;
    
    Mario::large = large;
    Mario::fire = fire;
    
    newLarge = Mario::large;
    newFire = Mario::fire;
    
    blink(true);
}

void Mario::calcPic() {
    int runFrame = 0;
    
    if (large) {
        runFrame = ((int)(runTime / 20)) % 4;
        if (runFrame == 3) runFrame = 1;
        if (carried == nullptr && std::abs(xa) > 10) runFrame += 3;
        if (carried != nullptr) runFrame += 10;
        if (!onGround) {
            if (carried != nullptr) runFrame = 12;
            else if (std::abs(xa) > 10) runFrame = 7;
            else runFrame = 6;
        }
    } else {
        runFrame = ((int)(runTime / 20)) % 2;
        if (carried == nullptr && std::abs(xa) > 10) runFrame += 2;
        if (carried != nullptr) runFrame += 8;
        if (!onGround) {
            if (carried != nullptr) runFrame = 9;
            else if (std::abs(xa) > 10) runFrame = 5;
            else runFrame = 4;
        }
    }
    
    if (onGround && ((facing == -1 && xa > 0) || (facing == 1 && xa < 0))) {
        if (xa > 1 || xa < -1) runFrame = large ? 9 : 7;
    }
    
    if (large && ducking) runFrame = 14;
    
    // All mario sheets only have 1 row (yPic = 0)
    yPic = 0;
    
    xPic = runFrame;
}

void Mario::move() {
    if (winTime > 0) {
        winTime++;
        xa = 0;
        ya = 0;
        return;
    }
    
    if (deathTime > 0) {
        deathTime++;
        if (deathTime < 11) {
            xa = 0;
            ya = 0;
        } else if (deathTime == 11) {
            ya = -15;
        } else {
            ya += 2;
        }
        x += xa;
        y += ya;
        return;
    }
    
    if (powerUpTime != 0) {
        if (powerUpTime > 0) {
            powerUpTime--;
            blink(((powerUpTime / 3) & 1) == 0);
        } else {
            powerUpTime++;
            blink(((-powerUpTime / 3) & 1) == 0);
        }
        
        if (powerUpTime == 0) world->paused = false;
        calcPic();
        return;
    }
    
    if (invulnerableTime > 0) invulnerableTime--;
    visible = ((invulnerableTime / 2) & 1) == 0;
    
    wasOnGround = onGround;
    float sideWaysSpeed = keys[KEY_SPEED] ? 1.2f : 0.6f;
    
    if (onGround) {
        ducking = keys[KEY_DOWN] && large;
    }
    
    if (xa > 2) facing = 1;
    if (xa < -2) facing = -1;
    
    if (keys[KEY_JUMP] || (jumpTime < 0 && !onGround && !sliding)) {
        if (jumpTime < 0) {
            xa = xJumpSpeed;
            ya = -jumpTime * yJumpSpeed;
            jumpTime++;
        } else if (onGround && mayJump) {
            Art::playSound(SAMPLE_MARIO_JUMP);
            xJumpSpeed = 0;
            yJumpSpeed = -1.9f;
            jumpTime = 7;
            ya = jumpTime * yJumpSpeed;
            onGround = false;
            sliding = false;
        } else if (sliding && mayJump) {
            Art::playSound(SAMPLE_MARIO_JUMP);
            xJumpSpeed = -facing * 6.0f;
            yJumpSpeed = -2.0f;
            jumpTime = -6;
            xa = xJumpSpeed;
            ya = -jumpTime * yJumpSpeed;
            onGround = false;
            sliding = false;
            facing = -facing;
        } else if (jumpTime > 0) {
            xa += xJumpSpeed;
            ya = jumpTime * yJumpSpeed;
            jumpTime--;
        }
    } else {
        jumpTime = 0;
    }
    
    if (keys[KEY_LEFT] && !ducking) {
        if (facing == 1) sliding = false;
        xa -= sideWaysSpeed;
        if (jumpTime >= 0) facing = -1;
    }
    
    if (keys[KEY_RIGHT] && !ducking) {
        if (facing == -1) sliding = false;
        xa += sideWaysSpeed;
        if (jumpTime >= 0) facing = 1;
    }
    
    if ((!keys[KEY_LEFT] && !keys[KEY_RIGHT]) || ducking || ya < 0 || onGround) {
        sliding = false;
    }
    
    if (keys[KEY_SPEED] && canShoot && fire && world->fireballsOnScreen < 2) {
        Art::playSound(SAMPLE_MARIO_FIREBALL);
        world->addSprite(new Fireball(world, x + facing * 6, y - 20, facing));
    }
    
    canShoot = !keys[KEY_SPEED];
    mayJump = (onGround || sliding) && !keys[KEY_JUMP];
    xFlipPic = facing == -1;
    
    runTime += std::abs(xa) + 5;
    if (std::abs(xa) < 0.5f) {
        runTime = 0;
        xa = 0;
    }
    
    calcPic();
    
    if (sliding) {
        for (int i = 0; i < 1; i++) {
            world->addSprite(new Sparkle((int)(x + (rand() % 4) - 2) + facing * 8,
                                          (int)(y + (rand() % 4)) - 24,
                                          (float)((rand() % 200) / 100.0f - 1),
                                          (float)(rand() % 100) / 100.0f,
                                          0, 1, 5));
        }
        ya *= 0.5f;
    }
    
    onGround = false;
    moveImpl(xa, 0);
    moveImpl(0, ya);
    
    if (y > world->level->height * 16 + 16) {
        die();
    }
    
    if (x < 0) {
        x = 0;
        xa = 0;
    }
    
    if (x > world->level->width * 16) {
        x = world->level->width * 16;
        xa = 0;
    }
    
    ya *= 0.85f;
    if (onGround) {
        xa *= GROUND_INERTIA;
    } else {
        xa *= AIR_INERTIA;
    }
    
    if (!onGround) {
        ya += 3;
    }
    
    // Handle carried shell - must sync BOTH current and old positions
    // so shell interpolates in sync with Mario
    if (carried != nullptr) {
        // Current position based on Mario's current position
        carried->x = x + facing * 8;
        carried->y = y - 2;
        // Old position based on Mario's OLD position (for interpolation sync)
        carried->xOld = xOld + facing * 8;
        carried->yOld = yOld - 2;
        
        // Release shell if speed button released
        if (!keys[KEY_SPEED]) {
            carried->release(this);
            carried = nullptr;
        }
    }
    
    // Check for exit - just crossing the xExit line triggers win
    if (x > world->level->xExit * 16) {
        win();
    }
}

bool Mario::moveImpl(float xa, float ya) {
    while (xa > 8) {
        if (!moveImpl(8, 0)) return false;
        xa -= 8;
    }
    while (xa < -8) {
        if (!moveImpl(-8, 0)) return false;
        xa += 8;
    }
    while (ya > 8) {
        if (!moveImpl(0, 8)) return false;
        ya -= 8;
    }
    while (ya < -8) {
        if (!moveImpl(0, -8)) return false;
        ya += 8;
    }
    
    bool collide = false;
    
    if (ya > 0) {
        if (isBlocking(x + xa - width, y + ya, xa, 0)) collide = true;
        else if (isBlocking(x + xa + width, y + ya, xa, 0)) collide = true;
        else if (isBlocking(x + xa - width, y + ya + 1, xa, ya)) collide = true;
        else if (isBlocking(x + xa + width, y + ya + 1, xa, ya)) collide = true;
    }
    if (ya < 0) {
        if (isBlocking(x + xa, y + ya - height, xa, ya)) collide = true;
        else if (isBlocking(x + xa - width, y + ya - height, xa, ya)) collide = true;
        else if (isBlocking(x + xa + width, y + ya - height, xa, ya)) collide = true;
    }
    if (xa > 0) {
        sliding = true;
        if (isBlocking(x + xa + width, y + ya - height, xa, ya)) collide = true;
        else sliding = false;
        if (isBlocking(x + xa + width, y + ya - height / 2, xa, ya)) collide = true;
        else sliding = false;
        if (isBlocking(x + xa + width, y + ya, xa, ya)) collide = true;
        else sliding = false;
    }
    if (xa < 0) {
        sliding = true;
        if (isBlocking(x + xa - width, y + ya - height, xa, ya)) collide = true;
        else sliding = false;
        if (isBlocking(x + xa - width, y + ya - height / 2, xa, ya)) collide = true;
        else sliding = false;
        if (isBlocking(x + xa - width, y + ya, xa, ya)) collide = true;
        else sliding = false;
    }
    
    if (collide) {
        if (xa < 0) {
            x = (int)((x - width) / 16) * 16 + width;
            this->xa = 0;
        }
        if (xa > 0) {
            x = (int)((x + width) / 16 + 1) * 16 - width - 1;
            this->xa = 0;
        }
        if (ya < 0) {
            y = (int)((y - height) / 16) * 16 + height;
            jumpTime = 0;
            this->ya = 0;
        }
        if (ya > 0) {
            y = (int)((y - 1) / 16 + 1) * 16 - 1;
            onGround = true;
        }
        return false;
    } else {
        x += xa;
        y += ya;
        return true;
    }
}

bool Mario::isBlocking(float _x, float _y, float xa, float ya) {
    int tx = (int)(_x / 16);
    int ty = (int)(_y / 16);
    
    if (tx == (int)(x / 16) && ty == (int)(y / 16)) return false;
    
    bool blocking = world->level->isBlocking(tx, ty, xa, ya);
    
    uint8_t block = world->level->getBlock(tx, ty);
    
    if ((Level::TILE_BEHAVIORS[block] & Level::BIT_PICKUPABLE) > 0) {
        getCoin();
        Art::playSound(SAMPLE_GET_COIN);
        world->level->setBlock(tx, ty, 0);
        for (int xx = 0; xx < 2; xx++) {
            for (int yy = 0; yy < 2; yy++) {
                world->addSprite(new Sparkle(tx * 16 + xx * 8 + rand() % 8,
                                              ty * 16 + yy * 8 + rand() % 8,
                                              0, 0, 0, 2, 5));
            }
        }
    }
    
    if (blocking && ya < 0) {
        world->bump(tx, ty, large);
    }
    
    return blocking;
}

void Mario::stomp(Enemy* enemy) {
    if (deathTime > 0 || world->paused) return;
    
    float targetY = enemy->y - enemy->hPic / 2;
    moveImpl(0, targetY - y);
    
    Art::playSound(SAMPLE_MARIO_KICK);
    addScore(100);  // Points for stomping enemy
    xJumpSpeed = 0;
    yJumpSpeed = -1.9f;
    jumpTime = 8;
    ya = jumpTime * yJumpSpeed;
    onGround = false;
    sliding = false;
    invulnerableTime = 1;
}

void Mario::stomp(Shell* shell) {
    if (deathTime > 0 || world->paused) return;
    
    if (keys[KEY_SPEED] && shell->facing == 0) {
        carried = shell;
        shell->carried = true;
    } else {
        float targetY = shell->y - shell->hPic / 2;
        moveImpl(0, targetY - y);
        
        Art::playSound(SAMPLE_MARIO_KICK);
        xJumpSpeed = 0;
        yJumpSpeed = -1.9f;
        jumpTime = 8;
        ya = jumpTime * yJumpSpeed;
        onGround = false;
        sliding = false;
        invulnerableTime = 1;
    }
}

void Mario::stomp(BulletBill* bill) {
    if (deathTime > 0 || world->paused) return;
    
    float targetY = bill->y - bill->hPic / 2;
    moveImpl(0, targetY - y);
    
    Art::playSound(SAMPLE_MARIO_KICK);
    xJumpSpeed = 0;
    yJumpSpeed = -1.9f;
    jumpTime = 8;
    ya = jumpTime * yJumpSpeed;
    onGround = false;
    sliding = false;
    invulnerableTime = 1;
}

void Mario::kick(Shell* shell) {
    if (deathTime > 0 || world->paused) return;
    
    if (keys[KEY_SPEED]) {
        carried = shell;
        shell->carried = true;
    } else {
        Art::playSound(SAMPLE_MARIO_KICK);
        invulnerableTime = 1;
    }
}

/**
 * Called when Mario takes damage from enemies or hazards.
 * 
 * Behavior:
 * - If invulnerable (recently hit), ignore damage
 * - If large: Power down (lose fire or become small), gain brief invulnerability
 * - If small: Die
 * - In test mode with invincibility on: Completely invincible, no damage taken
 */
void Mario::getHurt() {
    if (deathTime > 0 || world->paused) return;
    if (invulnerableTime > 0) return;
    
    // Test mode invincibility (can be toggled with ` key)
    if (g_testMode && g_testInvincible) {
        DEBUG_PRINT("Mario would have been hurt (test mode - invincible)");
        return;
    }
    
    if (large) {
        world->paused = true;
        powerUpTime = -3 * 6;
        Art::playSound(SAMPLE_MARIO_POWER_DOWN);
        if (fire) {
            setLarge(true, false);
        } else {
            setLarge(false, false);
        }
        invulnerableTime = 32;
    } else {
        die();
    }
}

/**
 * Called when Mario dies (falls in pit, time runs out, hit while small).
 * In test mode with invincibility on, death is prevented.
 */
void Mario::die() {
    // Test mode invincibility (can be toggled with ` key)
    if (g_testMode && g_testInvincible) {
        DEBUG_PRINT("Mario would have died (test mode - invincible)");
        return;
    }
    
    xDeathPos = (int)x;
    yDeathPos = (int)y;
    setLarge(false, false);
    world->paused = true;
    deathTime = 1;
    Art::stopMusic();
    Art::playSound(SAMPLE_MARIO_DEATH);
}

void Mario::win() {
    xDeathPos = (int)x;
    yDeathPos = (int)y;
    world->paused = true;
    winTime = 1;
    Art::stopMusic();
    Art::playSound(SAMPLE_LEVEL_EXIT);
    
    // Convert all enemies to coins
    world->convertEnemiesToCoins();
}

void Mario::getFlower() {
    if (deathTime > 0 || world->paused) return;
    
    if (!fire && large) {
        world->paused = true;
        powerUpTime = 3 * 6;
        Art::playSound(SAMPLE_MARIO_POWER_UP);
        setLarge(true, true);
    } else if (!fire && !large) {
        getMushroom();
    } else {
        getCoin();
        Art::playSound(SAMPLE_GET_COIN);
    }
}

void Mario::getMushroom() {
    if (deathTime > 0 || world->paused) return;
    
    if (!large) {
        world->paused = true;
        powerUpTime = 3 * 6;
        Art::playSound(SAMPLE_MARIO_POWER_UP);
        setLarge(true, false);
    } else {
        getCoin();
        Art::playSound(SAMPLE_GET_COIN);
    }
}

void Mario::getOneUp() {
    if (deathTime > 0 || world->paused) return;
    get1Up();
}

uint8_t Mario::getKeyMask() const {
    uint8_t mask = 0;
    for (int i = 0; i < 7; i++) {
        if (keys[i]) mask |= (1 << i);
    }
    return mask;
}

void Mario::setKeys(uint8_t mask) {
    for (int i = 0; i < 7; i++) {
        keys[i] = (mask & (1 << i)) > 0;
    }
}

void Mario::render(SDL_Renderer* renderer, float alpha) {
    if (large) {
        height = ducking ? 12 : 24;
    } else {
        height = 12;
    }
    Sprite::render(renderer, alpha);
}

/**
 * @file LevelScene.cpp
 * @brief Main gameplay scene implementation.
 */
#include "LevelScene.h"
#include "Game.h"
#include "Art.h"
#include "Level.h"
#include "LevelGenerator.h"
#include "LevelRenderer.h"
#include "BgRenderer.h"
#include "Mario.h"
#include "Enemy.h"
#include "FlowerEnemy.h"
#include "Shell.h"
#include "Fireball.h"
#include "BulletBill.h"
#include "Mushroom.h"
#include "FireFlower.h"
#include "CoinAnim.h"
#include "Sparkle.h"
#include "Particle.h"
#include "SpriteTemplate.h"
#include <algorithm>
#include <cmath>

LevelScene::LevelScene(Game* game, long seed, int levelDifficulty, int type)
    : levelSeed(seed), levelDifficulty(levelDifficulty), levelType(type) {
    this->game = game;
}

LevelScene::~LevelScene() {
    for (auto* sprite : sprites) {
        delete sprite;
    }
    sprites.clear();
    
    delete level;
    delete layer;
    delete bgLayer[0];
    delete bgLayer[1];
}

void LevelScene::init() {
    DEBUG_PRINT("LevelScene::init() seed=%ld difficulty=%d type=%d", levelSeed, levelDifficulty, levelType);
    
    level = LevelGenerator::createLevel(320, 15, levelSeed, levelDifficulty, levelType);
    DEBUG_PRINT("  Level created: %dx%d", level->width, level->height);
    
    layer = new LevelRenderer(level, SCREEN_WIDTH, SCREEN_HEIGHT);
    // Create two background layers with different scroll speeds (distance)
    // Java: scrollSpeed = 4 >> i, so layer 0 has distance 4, layer 1 has distance 2
    bgLayer[0] = new BgRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, levelType, 4, true);   // distant
    bgLayer[1] = new BgRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, levelType, 2, false);  // near
    
    mario = new Mario(this);
    sprites.push_back(mario);
    DEBUG_PRINT("  Mario spawned at (%.0f, %.0f)", mario->x, mario->y);
    
    // Java: startTime = 1, then increments each tick
    // Used for iris opening animation
    startTime = 1;
    
    timeLeft = 200 * TICKS_PER_SECOND;
    
    // Start music based on level type (force restart from beginning)
    musicType = levelType;
    if (levelType == 0) Art::startMusic(MUSIC_OVERWORLD, true);
    else if (levelType == 1) Art::startMusic(MUSIC_UNDERGROUND, true);
    else Art::startMusic(MUSIC_CASTLE, true);
}

void LevelScene::tick() {
    // User pause - game is completely frozen (don't even increment tick count)
    if (userPaused) {
        return;
    }
    
    tickCount++;
    
    // Increment startTime for opening iris animation (like Java)
    if (startTime > 0) {
        startTime++;
    }
    
    if (paused) {
        // Still tick Mario during death/win animation
        if (mario) {
            mario->tick();
        
            // Check for scene transition after iris closes
            // Java: deathTime uses t*t*0.4, transition at t > 1800 means 320 - t*t*0.4 < -some threshold
            // Java: winTime uses t*t*0.2, transition at t > 900
            if (mario->deathTime > 0) {
                float t = mario->deathTime;
                if (t * t * 0.4f > 1800) {
                    game->levelFailed();
                    return;
                }
            }
            if (mario->winTime > 0) {
                float t = mario->winTime;
                if (t * t * 0.2f > 900) {
                    game->levelWon();
                    return;
                }
            }
        }
        return;
    }
    
    level->tick();
    
    // In test mode, time doesn't decrease
    if (!g_testMode) {
        timeLeft--;
        
        if (timeLeft == 0 && mario) {
            mario->die();
        }
    }
    
    // Update camera
    xCamO = xCam;
    yCamO = yCam;
    if (mario) {
        xCam = mario->x - SCREEN_WIDTH / 2;
        yCam = mario->y - SCREEN_HEIGHT / 2;
    }
    
    // Clamp camera
    if (xCam < 0) xCam = 0;
    if (yCam < 0) yCam = 0;
    if (xCam > level->width * 16 - SCREEN_WIDTH) xCam = level->width * 16 - SCREEN_WIDTH;
    if (yCam > level->height * 16 - SCREEN_HEIGHT) yCam = level->height * 16 - SCREEN_HEIGHT;
    
    // Spawn enemies from templates and check for cannons
    for (int x = (int)xCam / 16 - 1; x <= (int)(xCam + SCREEN_WIDTH) / 16 + 1; x++) {
        for (int y = (int)yCam / 16 - 1; y <= (int)(yCam + SCREEN_HEIGHT) / 16 + 1; y++) {
            int dir = 0;
            if (mario) {
                if (x * 16 + 8 > mario->x) dir = -1;
                else dir = 1;
            } else {
                dir = 1;
            }
            
            SpriteTemplate* st = level->getSpriteTemplate(x, y);
            if (st && st->lastVisibleTick != tickCount - 1 && !st->isDead) {
                // Only spawn if sprite doesn't exist or isn't in sprite list
                bool shouldSpawn = (st->sprite == nullptr);
                if (!shouldSpawn) {
                    // Check if sprite is still in our sprite list
                    shouldSpawn = true;
                    for (auto* s : sprites) {
                        if (s == st->sprite) {
                            shouldSpawn = false;
                            break;
                        }
                    }
                }
                if (shouldSpawn) {
                    st->spawn(this, x, y, dir);
                }
            }
            if (st) st->lastVisibleTick = tickCount;
            
            // Check for cannon blocks and spawn BulletBills
            if (dir != 0) {
                uint8_t b = level->getBlock(x, y);
                if ((Level::TILE_BEHAVIORS[b & 0xff] & Level::BIT_ANIMATED) > 0) {
                    // Check if it's a cannon block (column 12-15, row 0)
                    if ((b % 16) / 4 == 3 && b / 16 == 0) {
                        if ((tickCount - x * 2) % 100 == 0) {
                            // Spawn sparkles
                            for (int i = 0; i < 8; i++) {
                                addSprite(new Sparkle(x * 16 + 8, y * 16 + rand() % 16,
                                                      (float)(rand() % 100) / 100.0f * dir, 0, 0, 1, 5));
                            }
                            // Spawn BulletBill
                            addSprite(new BulletBill(this, x * 16 + 8 + dir * 8, y * 16 + 15, dir));
                            Art::playSound(SAMPLE_CANNON_FIRE);
                        }
                    }
                }
            }
        }
    }
    
    // Update sprites and remove offscreen ones
    fireballsOnScreen = 0;
    for (auto* sprite : sprites) {
        // Check if sprite is far offscreen (more than 64 pixels outside view)
        // Don't remove Mario
        if (sprite != mario) {
            float xd = sprite->x - xCam;
            float yd = sprite->y - yCam;
            if (xd < -64 || xd > SCREEN_WIDTH + 64 || yd < -64 || yd > SCREEN_HEIGHT + 64) {
                removeSprite(sprite);
                // Reset the sprite template so it can respawn when back in view
                if (sprite->spriteTemplate) {
                    sprite->spriteTemplate->sprite = nullptr;
                }
                continue;
            }
        }
        if (dynamic_cast<Fireball*>(sprite)) fireballsOnScreen++;
    }
    
    for (auto* sprite : sprites) {
        sprite->tick();
    }
    
    for (auto* sprite : sprites) {
        sprite->collideCheck();
    }
    
    // Check shell collisions - collect shells to check first
    std::vector<Shell*> shellsToCheck;
    for (auto* sprite : sprites) {
        if (auto* shell = dynamic_cast<Shell*>(sprite)) {
            if (shell->facing != 0 || mario->carried == shell) {
                shellsToCheck.push_back(shell);
            }
        }
    }
    
    for (auto* shell : shellsToCheck) {
        if (shell->dead) continue;
        for (auto* other : sprites) {
            if (other != shell && !shell->dead) {
                if (other->shellCollideCheck(shell)) {
                    // If Mario was carrying this shell and it hit something, drop it
                    if (mario->carried == shell && !shell->dead) {
                        mario->carried = nullptr;
                        shell->die();
                    }
                }
            }
        }
    }
    
    // Check fireball collisions
    for (auto* sprite : sprites) {
        if (auto* fireball = dynamic_cast<Fireball*>(sprite)) {
            if (fireball->dead) continue;
            for (auto* other : sprites) {
                if (other != fireball && other != mario && !fireball->dead) {
                    if (other->fireballCollideCheck(fireball)) {
                        fireball->die();
                        break;
                    }
                }
            }
        }
    }
    
    // Add pending sprites
    for (auto* sprite : spritesToAdd) {
        sprite->spriteContext = this;
        sprites.push_back(sprite);
    }
    spritesToAdd.clear();
    
    // Remove pending sprites
    for (auto* sprite : spritesToRemove) {
        auto it = std::find(sprites.begin(), sprites.end(), sprite);
        if (it != sprites.end()) {
            sprites.erase(it);
            if (sprite != mario) {
                delete sprite;
            }
        }
    }
    spritesToRemove.clear();
}

void LevelScene::render(SDL_Renderer* renderer, float alpha) {
    // Camera follows Mario's interpolated position (like Java)
    float xCamLerp, yCamLerp;
    if (mario) {
        xCamLerp = (mario->xOld + (mario->x - mario->xOld) * alpha) - SCREEN_WIDTH / 2;
        yCamLerp = (mario->yOld + (mario->y - mario->yOld) * alpha) - SCREEN_HEIGHT / 2;
    } else {
        xCamLerp = lerp(xCamO, xCam, alpha);
        yCamLerp = lerp(yCamO, yCam, alpha);
    }
    
    // Clamp camera
    if (xCamLerp < 0) xCamLerp = 0;
    if (yCamLerp < 0) yCamLerp = 0;
    if (xCamLerp > level->width * 16 - SCREEN_WIDTH) xCamLerp = level->width * 16 - SCREEN_WIDTH;
    if (yCamLerp > level->height * 16 - SCREEN_HEIGHT) yCamLerp = level->height * 16 - SCREEN_HEIGHT;
    
    // Render backgrounds (order: far layer first, then near)
    if (bgLayer[0]) {
        bgLayer[0]->setCam((int)xCamLerp, (int)yCamLerp);
        bgLayer[0]->render(renderer, tickCount);
    }
    
    if (bgLayer[1]) {
        bgLayer[1]->setCam((int)xCamLerp, (int)yCamLerp);
        bgLayer[1]->render(renderer, tickCount);
    }
    
    // Render sprites
    // Layer 0 - behind level tiles (e.g. flowers in pipes)
    for (auto* sprite : sprites) {
        if (sprite->layer == 0) {
            // Transform to camera space
            float oldX = sprite->xOld;
            float oldY = sprite->yOld;
            float curX = sprite->x;
            float curY = sprite->y;
            
            sprite->xOld -= xCamLerp;
            sprite->yOld -= yCamLerp;
            sprite->x -= xCamLerp;
            sprite->y -= yCamLerp;
            
            sprite->render(renderer, alpha);
            
            // Restore original positions
            sprite->xOld = oldX;
            sprite->yOld = oldY;
            sprite->x = curX;
            sprite->y = curY;
        }
    }
    
    // Render exit (back part - left pole)
    if (layer) {
        layer->renderExit0(renderer, tickCount, alpha, mario == nullptr || mario->winTime == 0);
    }
    
    // Render level tiles on top of layer 0 sprites
    if (layer) {
        layer->xCam = (int)xCamLerp;
        layer->yCam = (int)yCamLerp;
        layer->render(renderer, tickCount);
    }
    
    // Layer 1 - in front of level tiles (most enemies, Mario, etc.)
    // First pass: render carried shell (so it appears behind Mario)
    if (mario && mario->carried) {
        Sprite* sprite = mario->carried;
        if (sprite->layer == 1) {
            float oldX = sprite->xOld;
            float oldY = sprite->yOld;
            float curX = sprite->x;
            float curY = sprite->y;
            
            sprite->xOld -= xCamLerp;
            sprite->yOld -= yCamLerp;
            sprite->x -= xCamLerp;
            sprite->y -= yCamLerp;
            
            sprite->render(renderer, alpha);
            
            sprite->xOld = oldX;
            sprite->yOld = oldY;
            sprite->x = curX;
            sprite->y = curY;
        }
    }
    
    // Second pass: render all other layer 1 sprites (skip carried shell to avoid double-render)
    for (auto* sprite : sprites) {
        if (sprite->layer == 1 && sprite != mario->carried) {
            int xPixel = (int)(sprite->xOld + (sprite->x - sprite->xOld) * alpha) - (int)xCamLerp;
            int yPixel = (int)(sprite->yOld + (sprite->y - sprite->yOld) * alpha) - (int)yCamLerp;
            
            // Transform to camera space
            float oldX = sprite->xOld;
            float oldY = sprite->yOld;
            float curX = sprite->x;
            float curY = sprite->y;
            
            sprite->xOld -= xCamLerp;
            sprite->yOld -= yCamLerp;
            sprite->x -= xCamLerp;
            sprite->y -= yCamLerp;
            
            sprite->render(renderer, alpha);
            
            sprite->xOld = oldX;
            sprite->yOld = oldY;
            sprite->x = curX;
            sprite->y = curY;
        }
    }
    
    // Render exit (front part - right pole)
    if (layer) {
        layer->renderExit1(renderer, tickCount, alpha);
    }
    
    // Render HUD - match Java positions exactly (character positions * 8 pixels)
    // Row 0: labels, Row 1: values (row * 8 pixels)
    char buf[64];
    
    // TUX lives at col 0
    snprintf(buf, sizeof(buf), "TUX %d", Mario::lives);
    Art::drawString(buf, 0 * 8, 0 * 8, 7);
    // Score at col 0, row 1
    snprintf(buf, sizeof(buf), "%08d", Mario::score);
    Art::drawString(buf, 0 * 8, 1 * 8, 7);
    
    // COIN at col 14
    Art::drawString("COIN", 14 * 8, 0 * 8, 7);
    snprintf(buf, sizeof(buf), " %02d", Mario::coins);
    Art::drawString(buf, 14 * 8, 1 * 8, 7);
    
    // WORLD at col 24
    Art::drawString("WORLD", 24 * 8, 0 * 8, 7);
    Art::drawString(" " + Mario::levelString, 24 * 8, 1 * 8, 7);
    
    // TIME at col 35
    int seconds = timeLeft / TICKS_PER_SECOND;
    if (seconds < 0) seconds = 0;
    Art::drawString("TIME", 35 * 8, 0 * 8, 7);
    snprintf(buf, sizeof(buf), " %03d", seconds);
    Art::drawString(buf, 35 * 8, 1 * 8, 7);
    
    // Draw PAUSE text centered on screen
    if (userPaused) {
        // Center "PAUSE" on screen (5 chars * 8 pixels = 40 pixels wide)
        // Screen is 320 wide, so start at (320-40)/2 = 140
        // Screen is 240 tall, center at 120 - 4 = 116
        Art::drawString("PAUSE", (SCREEN_WIDTH - 5 * 8) / 2, SCREEN_HEIGHT / 2 - 4, 7);
    }
    
    // Render iris wipe (blackout) effects - matching Java implementation
    // Opening iris at level start: centered on screen, radius grows
    if (startTime > 0) {
        float t = startTime + alpha - 2;
        t = t * t * 0.6f;
        renderBlackout(renderer, 160, 120, (int)t);
    }
    
    // Closing iris on win: centered on Mario's death position, radius shrinks
    if (mario && mario->winTime > 0) {
        float t = mario->winTime + alpha;
        t = t * t * 0.2f;
        renderBlackout(renderer, (int)(mario->xDeathPos - xCam), (int)(mario->yDeathPos - yCam), (int)(320 - t));
    }
    
    // Closing iris on death: centered on Mario's death position, radius shrinks
    if (mario && mario->deathTime > 0) {
        float t = mario->deathTime + alpha;
        t = t * t * 0.4f;
        renderBlackout(renderer, (int)(mario->xDeathPos - xCam), (int)(mario->yDeathPos - yCam), (int)(320 - t));
    }
}

void LevelScene::addSprite(Sprite* sprite) {
    sprite->spriteContext = this;
    spritesToAdd.push_back(sprite);
}

void LevelScene::removeSprite(Sprite* sprite) {
    spritesToRemove.push_back(sprite);
}

void LevelScene::bump(int x, int y, bool canBreakBricks) {
    uint8_t block = level->getBlock(x, y);
    
    if ((Level::TILE_BEHAVIORS[block] & Level::BIT_BUMPABLE) > 0) {
        bumpInto(x, y - 1);
        level->setBlock(x, y, 4); // Bumped block
        level->setBlockData(x, y, 4);
        
        if ((Level::TILE_BEHAVIORS[block] & Level::BIT_SPECIAL) > 0) {
            Art::playSound(SAMPLE_ITEM_SPROUT);
            if (!Mario::large) {
                addSprite(new Mushroom(this, x * 16 + 8, y * 16 + 8));
            } else {
                addSprite(new FireFlower(this, x * 16 + 8, y * 16 + 8));
            }
        } else {
            Mario::getCoin();
            Art::playSound(SAMPLE_GET_COIN);
            addSprite(new CoinAnim(x, y));  // Pass tile coordinates
        }
    }
    
    if ((Level::TILE_BEHAVIORS[block] & Level::BIT_BREAKABLE) > 0) {
        bumpInto(x, y - 1);
        if (canBreakBricks) {
            level->setBlock(x, y, 0);
            Art::playSound(SAMPLE_BREAK_BLOCK);
            
            for (int xx = 0; xx < 2; xx++) {
                for (int yy = 0; yy < 2; yy++) {
                    addSprite(new Particle(x * 16 + xx * 8 + 4, y * 16 + yy * 8 + 4,
                                          (xx * 2 - 1) * 4.0f, (yy * 2 - 1) * 4.0f - 8));
                }
            }
        } else {
            // Small Mario bumps but doesn't break
            level->setBlockData(x, y, 4);
            Art::playSound(SAMPLE_SHELL_BUMP);
        }
    }
}

void LevelScene::bumpInto(int x, int y) {
    uint8_t block = level->getBlock(x, y);
    if ((Level::TILE_BEHAVIORS[block] & Level::BIT_PICKUPABLE) > 0) {
        Mario::getCoin();
        Art::playSound(SAMPLE_GET_COIN);
        level->setBlock(x, y, 0);
    }
    
    for (auto* sprite : sprites) {
        sprite->bumpCheck(x, y);
    }
}

void LevelScene::checkShellCollide(Sprite* shell) {
    // Collisions are handled inline in tick() for now
    // This function exists for compatibility
}

void LevelScene::checkFireballCollide(Sprite* fireball) {
    // Collisions are handled inline in tick() for now
    // This function exists for compatibility
}

/**
 * Handle test mode key presses for debugging and testing.
 * 
 * Mario state keys:
 *   'i' - Set to small Mario
 *   'o' - Set to big Mario (no fire)
 *   'p' - Set to fire Mario
 * 
 * Enemy spawn keys (spawns at Mario's position + offset):
 *   '0' - Red Koopa
 *   '1' - Green Koopa
 *   '2' - Goomba
 *   '3' - Spiky
 *   '4' - Piranha Plant (FlowerEnemy)
 *   '5' - Winged Red Koopa
 *   '6' - Winged Green Koopa
 *   '7' - Winged Goomba
 *   '8' - BulletBill
 *   '9' - Shell
 */
void LevelScene::handleTestKey(char key) {
    if (!mario) return;
    
    switch (key) {
        // === MARIO STATE CHANGES ===
        case 'i':
            DEBUG_PRINT("Test: Setting Mario to small");
            mario->setLarge(false, false);
            break;
        case 'o':
            DEBUG_PRINT("Test: Setting Mario to big (no fire)");
            mario->setLarge(true, false);
            break;
        case 'p':
            DEBUG_PRINT("Test: Setting Mario to fire");
            mario->setLarge(true, true);
            break;
            
        // === ENEMY SPAWNING ===
        // Enemies spawn 32 pixels to the right of Mario
        case '0': {
            DEBUG_PRINT("Test: Spawning Red Koopa");
            Enemy* enemy = new Enemy(this, (int)mario->x + 32, (int)mario->y, -1, Enemy::ENEMY_RED_KOOPA, false);
            addSprite(enemy);
            break;
        }
        case '1': {
            DEBUG_PRINT("Test: Spawning Green Koopa");
            Enemy* enemy = new Enemy(this, (int)mario->x + 32, (int)mario->y, -1, Enemy::ENEMY_GREEN_KOOPA, false);
            addSprite(enemy);
            break;
        }
        case '2': {
            DEBUG_PRINT("Test: Spawning Goomba");
            Enemy* enemy = new Enemy(this, (int)mario->x + 32, (int)mario->y, -1, Enemy::ENEMY_GOOMBA, false);
            addSprite(enemy);
            break;
        }
        case '3': {
            DEBUG_PRINT("Test: Spawning Spiky");
            Enemy* enemy = new Enemy(this, (int)mario->x + 32, (int)mario->y, -1, Enemy::ENEMY_SPIKY, false);
            addSprite(enemy);
            break;
        }
        case '4': {
            DEBUG_PRINT("Test: Spawning Piranha Plant");
            FlowerEnemy* flower = new FlowerEnemy(this, (int)mario->x + 32, (int)mario->y);
            addSprite(flower);
            break;
        }
        case '5': {
            DEBUG_PRINT("Test: Spawning Winged Red Koopa");
            Enemy* enemy = new Enemy(this, (int)mario->x + 32, (int)mario->y, -1, Enemy::ENEMY_RED_KOOPA, true);
            addSprite(enemy);
            break;
        }
        case '6': {
            DEBUG_PRINT("Test: Spawning Winged Green Koopa");
            Enemy* enemy = new Enemy(this, (int)mario->x + 32, (int)mario->y, -1, Enemy::ENEMY_GREEN_KOOPA, true);
            addSprite(enemy);
            break;
        }
        case '7': {
            DEBUG_PRINT("Test: Spawning Winged Goomba");
            Enemy* enemy = new Enemy(this, (int)mario->x + 32, (int)mario->y, -1, Enemy::ENEMY_GOOMBA, true);
            addSprite(enemy);
            break;
        }
        case '8': {
            DEBUG_PRINT("Test: Spawning BulletBill");
            BulletBill* bill = new BulletBill(this, (int)mario->x + 32, (int)mario->y, -1);
            addSprite(bill);
            break;
        }
        case '9': {
            DEBUG_PRINT("Test: Spawning Shell");
            Shell* shell = new Shell(this, (int)mario->x + 32, (int)mario->y, 1);
            addSprite(shell);
            break;
        }
    }
}

/**
 * Handle pause key press (Enter).
 * Toggles the user pause state - when paused, the game freezes,
 * music pauses, and "PAUSE" is displayed in the center of the screen.
 * 
 * Note: Does not pause during death/win animations (when 'paused' is true).
 */
void LevelScene::handlePauseKey() {
    // Don't allow pause toggle during death/win animations
    if (paused) return;
    
    userPaused = !userPaused;
    DEBUG_PRINT("Game %s", userPaused ? "PAUSED" : "RESUMED");
    
    // Pause/resume music
    if (userPaused) {
        Mix_PauseMusic();
    } else {
        Mix_ResumeMusic();
    }
}

/**
 * Render the iris wipe (blackout) effect.
 * This is a direct port of the Java renderBlackout method.
 * 
 * Draws two black polygons (top half and bottom half of the screen)
 * with a circular cutout centered at (x, y) with given radius.
 * 
 * @param x Center X of the circular opening
 * @param y Center Y of the circular opening  
 * @param radius Radius of the circular opening (0 = fully black, >320 = fully visible)
 */
void LevelScene::renderBlackout(SDL_Renderer* renderer, int x, int y, int radius) {
    // If radius is large enough, nothing to draw
    if (radius > 320) return;
    
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    
    // If radius is 0 or negative, fill entire screen with black
    if (radius <= 0) {
        SDL_Rect fullScreen = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderFillRect(renderer, &fullScreen);
        return;
    }
    
    // Java uses polygons with 20 points to draw the blackout
    // We'll approximate this with SDL by drawing filled shapes
    // The Java code draws two semi-circular masks (top and bottom halves)
    
    // For SDL, we'll use a scanline approach since SDL doesn't have fillPolygon
    // Draw horizontal lines for the top half (above center line)
    for (int row = 0; row < y; row++) {
        // Calculate the x extent of the circle at this row
        int dy = y - row;
        if (dy > radius) {
            // Entire row is outside circle - fill it
            SDL_Rect line = {0, row, SCREEN_WIDTH, 1};
            SDL_RenderFillRect(renderer, &line);
        } else {
            // Calculate where circle edge is
            float dx = std::sqrt((float)(radius * radius - dy * dy));
            int leftEdge = x - (int)dx;
            int rightEdge = x + (int)dx;
            
            // Fill left side
            if (leftEdge > 0) {
                SDL_Rect left = {0, row, leftEdge, 1};
                SDL_RenderFillRect(renderer, &left);
            }
            // Fill right side
            if (rightEdge < SCREEN_WIDTH) {
                SDL_Rect right = {rightEdge, row, SCREEN_WIDTH - rightEdge, 1};
                SDL_RenderFillRect(renderer, &right);
            }
        }
    }
    
    // Draw horizontal lines for the bottom half (below center line)
    for (int row = y; row < SCREEN_HEIGHT; row++) {
        int dy = row - y;
        if (dy > radius) {
            // Entire row is outside circle - fill it
            SDL_Rect line = {0, row, SCREEN_WIDTH, 1};
            SDL_RenderFillRect(renderer, &line);
        } else {
            // Calculate where circle edge is
            float dx = std::sqrt((float)(radius * radius - dy * dy));
            int leftEdge = x - (int)dx;
            int rightEdge = x + (int)dx;
            
            // Fill left side
            if (leftEdge > 0) {
                SDL_Rect left = {0, row, leftEdge, 1};
                SDL_RenderFillRect(renderer, &left);
            }
            // Fill right side
            if (rightEdge < SCREEN_WIDTH) {
                SDL_Rect right = {rightEdge, row, SCREEN_WIDTH - rightEdge, 1};
                SDL_RenderFillRect(renderer, &right);
            }
        }
    }
}

/**
 * Convert all enemies to coins when Mario wins the level.
 * This includes: regular enemies, piranha plants, shells, and bullet bills.
 * Each enemy spawns a coin animation and gives Mario coins/score.
 */
void LevelScene::convertEnemiesToCoins() {
    std::vector<Sprite*> enemiesToRemove;
    
    for (auto* sprite : sprites) {
        bool isEnemy = false;
        float enemyX = 0, enemyY = 0;
        
        // Check for regular enemies (goombas, koopas, spikies)
        if (auto* enemy = dynamic_cast<Enemy*>(sprite)) {
            isEnemy = true;
            enemyX = enemy->x;
            enemyY = enemy->y;
        }
        // Check for piranha plants
        else if (auto* flower = dynamic_cast<FlowerEnemy*>(sprite)) {
            isEnemy = true;
            enemyX = flower->x;
            enemyY = flower->y;
        }
        // Check for shells (including carried shell)
        else if (auto* shell = dynamic_cast<Shell*>(sprite)) {
            isEnemy = true;
            enemyX = shell->x;
            enemyY = shell->y;
        }
        // Check for bullet bills
        else if (auto* bill = dynamic_cast<BulletBill*>(sprite)) {
            isEnemy = true;
            enemyX = bill->x;
            enemyY = bill->y;
        }
        
        if (isEnemy) {
            // Convert pixel position to tile position for CoinAnim
            int tileX = (int)enemyX / 16;
            int tileY = (int)enemyY / 16;
            
            // Add coin animation at enemy position
            addSprite(new CoinAnim(tileX, tileY));
            
            // Give Mario the coin reward
            Mario::getCoin();
            
            // Mark enemy for removal
            enemiesToRemove.push_back(sprite);
        }
    }
    
    // Clear carried shell reference if it was converted
    if (mario && mario->carried) {
        for (auto* sprite : enemiesToRemove) {
            if (sprite == mario->carried) {
                mario->carried = nullptr;
                break;
            }
        }
    }
    
    // Remove all converted enemies
    for (auto* sprite : enemiesToRemove) {
        removeSprite(sprite);
    }
}

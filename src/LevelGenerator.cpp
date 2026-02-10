/**
 * @file LevelGenerator.cpp
 * @brief Procedural level generation implementation.
 */
#include "LevelGenerator.h"
#include "Level.h"
#include "SpriteTemplate.h"
#include "Enemy.h"
#include <algorithm>
#include <vector>

// Constants matching Java
const int LevelGenerator::TYPE_OVERGROUND = 0;
const int LevelGenerator::TYPE_UNDERGROUND = 1;
const int LevelGenerator::TYPE_CASTLE = 2;

const int LevelGenerator::ODDS_STRAIGHT = 0;
const int LevelGenerator::ODDS_HILL_STRAIGHT = 1;
const int LevelGenerator::ODDS_TUBES = 2;
const int LevelGenerator::ODDS_JUMP = 3;
const int LevelGenerator::ODDS_CANNONS = 4;

Level* LevelGenerator::createLevel(int width, int height, long seed, int difficulty, int type) {
    LevelGenerator gen(width, height);
    return gen.createLevelInternal(seed, difficulty, type);
}

Level* LevelGenerator::createBgLevel(int width, int height, bool distant, int type) {
    static Random bgRandom;
    Level* level = new Level(width, height);
    Random random(bgRandom.nextLong());
    
    switch (type) {
        case TYPE_OVERGROUND:
        {
            int range = distant ? 4 : 6;
            int offs = distant ? 2 : 1;
            int oh = random.nextInt(range) + offs;
            int h = random.nextInt(range) + offs;
            
            for (int x = 0; x < width; x++) {
                oh = h;
                while (oh == h) {
                    h = random.nextInt(range) + offs;
                }
                for (int y = 0; y < height; y++) {
                    int h0 = (oh < h) ? oh : h;
                    int h1 = (oh < h) ? h : oh;
                    
                    if (y < h0) {
                        if (distant) {
                            int s = 2;
                            if (y < 2) s = y;
                            level->setBlock(x, y, (uint8_t)(4 + s * 8));
                        } else {
                            level->setBlock(x, y, (uint8_t)5);
                        }
                    } else if (y == h0) {
                        int s = (h0 == h) ? 0 : 1;
                        s += distant ? 2 : 0;
                        level->setBlock(x, y, (uint8_t)s);
                    } else if (y == h1) {
                        int s = (h0 == h) ? 0 : 1;
                        s += distant ? 2 : 0;
                        level->setBlock(x, y, (uint8_t)(s + 16));
                    }
                }
            }
            break;
        }
        case TYPE_UNDERGROUND:
        {
            // Underground background - simplified
            for (int x = 0; x < width; x++) {
                for (int y = 0; y < height; y++) {
                    if (y < 2) {
                        level->setBlock(x, y, (uint8_t)4);
                    }
                }
            }
            break;
        }
        case TYPE_CASTLE:
        {
            // Castle background - simplified
            for (int x = 0; x < width; x++) {
                for (int y = 0; y < height; y++) {
                    if (y < 2) {
                        level->setBlock(x, y, (uint8_t)4);
                    }
                }
            }
            break;
        }
    }
    
    return level;
}

LevelGenerator::LevelGenerator(int width, int height) : width(width), height(height) {
    for (int i = 0; i < 5; i++) odds[i] = 0;
}

Level* LevelGenerator::createLevelInternal(long seed, int difficulty, int type) {
    this->type = type;
    this->difficulty = difficulty;
    totalOdds = 0;
    
    odds[ODDS_STRAIGHT] = 20;
    odds[ODDS_HILL_STRAIGHT] = 10;
    odds[ODDS_TUBES] = 2 + difficulty;
    odds[ODDS_JUMP] = 2 * difficulty;
    odds[ODDS_CANNONS] = -10 + 5 * difficulty;
    
    if (type != TYPE_OVERGROUND) {
        odds[ODDS_HILL_STRAIGHT] = 0;
    }
    
    for (int i = 0; i < 5; i++) {
        if (odds[i] < 0) odds[i] = 0;
        totalOdds += odds[i];
        odds[i] = totalOdds - odds[i];
    }
    
    level = new Level(width, height);
    random = Random(seed);
    
    int length = 0;
    length += buildStraight(0, level->width, true);
    while (length < level->width - 64) {
        length += buildZone(length, level->width - length);
    }
    
    int floor = height - 1 - random.nextInt(4);
    
    level->xExit = length + 8;
    level->yExit = floor;
    
    // Fill exit area with ground
    for (int x = length; x < level->width; x++) {
        for (int y = 0; y < height; y++) {
            if (y >= floor) {
                level->setBlock(x, y, (uint8_t)(1 + 9 * 16));
            }
        }
    }
    
    // Add ceiling for castle/underground
    if (type == TYPE_CASTLE || type == TYPE_UNDERGROUND) {
        int ceiling = 0;
        int run = 0;
        for (int x = 0; x < level->width; x++) {
            if (run-- <= 0 && x > 4) {
                ceiling = random.nextInt(4);
                run = random.nextInt(4) + 4;
            }
            for (int y = 0; y < level->height; y++) {
                if ((x > 4 && y <= ceiling) || x < 1) {
                    level->setBlock(x, y, (uint8_t)(1 + 9 * 16));
                }
            }
        }
    }
    
    fixWalls();
    
    return level;
}

int LevelGenerator::buildZone(int x, int maxLength) {
    int t = random.nextInt(totalOdds);
    int zoneType = 0;
    for (int i = 0; i < 5; i++) {
        if (odds[i] <= t) {
            zoneType = i;
        }
    }
    
    switch (zoneType) {
        case ODDS_STRAIGHT:
            return buildStraight(x, maxLength, false);
        case ODDS_HILL_STRAIGHT:
            return buildHillStraight(x, maxLength);
        case ODDS_TUBES:
            return buildTubes(x, maxLength);
        case ODDS_JUMP:
            return buildJump(x, maxLength);
        case ODDS_CANNONS:
            return buildCannons(x, maxLength);
    }
    return 0;
}

int LevelGenerator::buildStraight(int xo, int maxLength, bool safe) {
    int length = random.nextInt(10) + 2;
    if (safe) length = 10 + random.nextInt(5);
    if (length > maxLength) length = maxLength;
    
    int floor = height - 1 - random.nextInt(4);
    for (int x = xo; x < xo + length; x++) {
        for (int y = 0; y < height; y++) {
            if (y >= floor) {
                level->setBlock(x, y, (uint8_t)(1 + 9 * 16));
            }
        }
    }
    
    if (!safe) {
        if (length > 5) {
            decorate(xo, xo + length, floor);
        }
    }
    
    return length;
}

int LevelGenerator::buildHillStraight(int xo, int maxLength) {
    int length = random.nextInt(10) + 10;
    if (length > maxLength) length = maxLength;
    
    int floor = height - 1 - random.nextInt(4);
    for (int x = xo; x < xo + length; x++) {
        for (int y = 0; y < height; y++) {
            if (y >= floor) {
                level->setBlock(x, y, (uint8_t)(1 + 9 * 16));
            }
        }
    }
    
    addEnemyLine(xo + 1, xo + length - 1, floor - 1);
    
    int h = floor;
    bool keepGoing = true;
    std::vector<bool> occupied(length, false);
    
    while (keepGoing) {
        h = h - 2 - random.nextInt(3);
        if (h <= 0) {
            keepGoing = false;
        } else {
            int l = random.nextInt(5) + 3;
            int xxo = random.nextInt(length - l - 2) + xo + 1;
            
            if (xxo - xo < 0 || xxo - xo >= length || xxo - xo + l >= length ||
                occupied[xxo - xo] || occupied[xxo - xo + l] || 
                (xxo - xo - 1 >= 0 && occupied[xxo - xo - 1]) || 
                (xxo - xo + l + 1 < length && occupied[xxo - xo + l + 1])) {
                keepGoing = false;
            } else {
                occupied[xxo - xo] = true;
                occupied[xxo - xo + l] = true;
                addEnemyLine(xxo, xxo + l, h - 1);
                if (random.nextInt(4) == 0) {
                    decorate(xxo - 1, xxo + l + 1, h);
                    keepGoing = false;
                }
                for (int x = xxo; x < xxo + l; x++) {
                    for (int y = h; y < floor; y++) {
                        int xx = 5;
                        if (x == xxo) xx = 4;
                        if (x == xxo + l - 1) xx = 6;
                        int yy = 9;
                        if (y == h) yy = 8;
                        
                        if (level->getBlock(x, y) == 0) {
                            level->setBlock(x, y, (uint8_t)(xx + yy * 16));
                        } else {
                            if (level->getBlock(x, y) == (uint8_t)(4 + 8 * 16)) 
                                level->setBlock(x, y, (uint8_t)(4 + 11 * 16));
                            if (level->getBlock(x, y) == (uint8_t)(6 + 8 * 16)) 
                                level->setBlock(x, y, (uint8_t)(6 + 11 * 16));
                        }
                    }
                }
            }
        }
    }
    
    return length;
}

int LevelGenerator::buildTubes(int xo, int maxLength) {
    int length = random.nextInt(10) + 5;
    if (length > maxLength) length = maxLength;
    
    int floor = height - 1 - random.nextInt(4);
    int xTube = xo + 1 + random.nextInt(4);
    int tubeHeight = floor - random.nextInt(2) - 2;
    
    for (int x = xo; x < xo + length; x++) {
        if (x > xTube + 1) {
            xTube += 3 + random.nextInt(4);
            tubeHeight = floor - random.nextInt(2) - 2;
        }
        if (xTube >= xo + length - 2) xTube += 10;
        
        if (x == xTube && random.nextInt(11) < difficulty + 1) {
            // Add flower enemy in tube
            SpriteTemplate* st = new SpriteTemplate(Enemy::ENEMY_FLOWER, false);
            level->setSpriteTemplate(x, tubeHeight, st);
        }
        
        for (int y = 0; y < height; y++) {
            if (y >= floor) {
                level->setBlock(x, y, (uint8_t)(1 + 9 * 16));
            } else {
                if ((x == xTube || x == xTube + 1) && y >= tubeHeight) {
                    int xPic = 10 + x - xTube;
                    if (y == tubeHeight) {
                        level->setBlock(x, y, (uint8_t)(xPic + 0 * 16));
                    } else {
                        level->setBlock(x, y, (uint8_t)(xPic + 1 * 16));
                    }
                }
            }
        }
    }
    
    return length;
}

int LevelGenerator::buildJump(int xo, int maxLength) {
    int js = random.nextInt(4) + 2;
    int jl = random.nextInt(2) + 2;
    int length = js * 2 + jl;
    
    if (length > maxLength) length = maxLength;
    
    bool hasStairs = random.nextInt(3) == 0;
    
    int floor = height - 1 - random.nextInt(4);
    for (int x = xo; x < xo + length; x++) {
        if (x < xo + js || x > xo + length - js - 1) {
            for (int y = 0; y < height; y++) {
                if (y >= floor) {
                    level->setBlock(x, y, (uint8_t)(1 + 9 * 16));
                } else if (hasStairs) {
                    if (x < xo + js) {
                        if (y >= floor - (x - xo) + 1) {
                            level->setBlock(x, y, (uint8_t)(9 + 0 * 16));
                        }
                    } else {
                        if (y >= floor - (xo + length - 1 - x) + 1) {
                            level->setBlock(x, y, (uint8_t)(9 + 0 * 16));
                        }
                    }
                }
            }
        }
    }
    
    return length;
}

int LevelGenerator::buildCannons(int xo, int maxLength) {
    int length = random.nextInt(10) + 2;
    if (length > maxLength) length = maxLength;
    
    int floor = height - 1 - random.nextInt(4);
    int xCannon = xo + 1 + random.nextInt(4);
    
    for (int x = xo; x < xo + length; x++) {
        if (x > xCannon) {
            xCannon += 2 + random.nextInt(4);
        }
        if (xCannon >= xo + length - 2) xCannon += 10;
        
        int cannonHeight = floor - random.nextInt(4) - 1;
        
        for (int y = 0; y < height; y++) {
            if (y >= floor) {
                level->setBlock(x, y, (uint8_t)(1 + 9 * 16));
            } else {
                if (x == xCannon) {
                    if (y >= cannonHeight) {
                        if (y == cannonHeight) {
                            level->setBlock(x, y, (uint8_t)(14 + 0 * 16));
                        } else if (y == cannonHeight + 1) {
                            level->setBlock(x, y, (uint8_t)(14 + 1 * 16));
                        } else {
                            level->setBlock(x, y, (uint8_t)(14 + 2 * 16));
                        }
                    }
                }
            }
        }
    }
    
    return length;
}

void LevelGenerator::decorate(int x0, int x1, int floor) {
    if (floor < 1) return;
    
    // boolean rocks = true; (always true in Java)
    bool rocks = true;
    
    addEnemyLine(x0 + 1, x1 - 1, floor - 1);
    
    int s = random.nextInt(4);
    int e = random.nextInt(4);
    
    // Add coin row at floor - 2
    if (floor - 2 > 0) {
        if ((x1 - 1 - e) - (x0 + 1 + s) > 1) {
            for (int x = x0 + 1 + s; x < x1 - 1 - e; x++) {
                level->setBlock(x, floor - 2, (uint8_t)(2 + 2 * 16));
            }
        }
    }
    
    s = random.nextInt(4);
    e = random.nextInt(4);
    
    // Add blocks at floor - 4 (matching Java logic exactly)
    if (floor - 4 > 0) {
        if ((x1 - 1 - e) - (x0 + 1 + s) > 2) {
            for (int x = x0 + 1 + s; x < x1 - 1 - e; x++) {
                if (rocks) {
                    // Java: Only non-edge blocks have 1/3 chance of being special (? blocks)
                    // This is the KEY condition that was missing!
                    if (x != x0 + 1 && x != x1 - 2 && random.nextInt(3) == 0) {
                        // Special block - 1/4 chance powerup, 3/4 chance coin
                        if (random.nextInt(4) == 0) {
                            level->setBlock(x, floor - 4, (uint8_t)(4 + 2 + 1 * 16));  // Powerup block
                        } else {
                            level->setBlock(x, floor - 4, (uint8_t)(4 + 1 + 1 * 16));  // Coin block
                        }
                    } else if (random.nextInt(4) == 0) {
                        // Hidden block - 1/4 chance powerup, 3/4 chance coin
                        if (random.nextInt(4) == 0) {
                            level->setBlock(x, floor - 4, (uint8_t)(2 + 1 * 16));  // Hidden powerup
                        } else {
                            level->setBlock(x, floor - 4, (uint8_t)(1 + 1 * 16));  // Hidden coin
                        }
                    } else {
                        // Regular brick
                        level->setBlock(x, floor - 4, (uint8_t)(0 + 1 * 16));  // Brick
                    }
                }
            }
        }
    }
}

void LevelGenerator::addEnemyLine(int x0, int x1, int y) {
    for (int x = x0; x < x1; x++) {
        if (random.nextInt(35) < difficulty + 1) {
            int enemyType = random.nextInt(4);
            if (difficulty < 1) {
                enemyType = Enemy::ENEMY_GOOMBA;
            } else if (difficulty < 3) {
                enemyType = random.nextInt(3);
            }
            SpriteTemplate* st = new SpriteTemplate(enemyType, random.nextInt(35) < difficulty);
            level->setSpriteTemplate(x, y, st);
        }
    }
}

void LevelGenerator::fixWalls() {
    // Create a boolean map indicating which positions are filled with ground
    std::vector<std::vector<bool>> blockMap(width + 1, std::vector<bool>(height + 1, false));
    
    for (int x = 0; x <= width; x++) {
        for (int y = 0; y <= height; y++) {
            int blocks = 0;
            for (int xx = x - 1; xx < x + 1; xx++) {
                for (int yy = y - 1; yy < y + 1; yy++) {
                    if (level->getBlockCapped(xx, yy) == (uint8_t)(1 + 9 * 16)) {
                        blocks++;
                    }
                }
            }
            blockMap[x][y] = (blocks == 4);
        }
    }
    
    // Determine tile offset based on level type
    int to = 0;
    if (type == TYPE_CASTLE) {
        to = 4 * 2;  // 8
    } else if (type == TYPE_UNDERGROUND) {
        to = 4 * 3;  // 12
    }
    
    // Apply edge/corner tiles based on surrounding blocks
    bool b[2][2];
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            for (int xx = x; xx <= x + 1; xx++) {
                for (int yy = y; yy <= y + 1; yy++) {
                    int _xx = xx;
                    int _yy = yy;
                    if (_xx < 0) _xx = 0;
                    if (_yy < 0) _yy = 0;
                    if (_xx > width) _xx = width;
                    if (_yy > height) _yy = height;
                    b[xx - x][yy - y] = blockMap[_xx][_yy];
                }
            }
            
            if (b[0][0] == b[1][0] && b[0][1] == b[1][1]) {
                if (b[0][0] == b[0][1]) {
                    if (b[0][0]) {
                        level->setBlock(x, y, (uint8_t)(1 + 9 * 16 + to));  // Center fill
                    }
                    // else: keep old block
                } else {
                    if (b[0][0]) {
                        level->setBlock(x, y, (uint8_t)(1 + 10 * 16 + to));  // Bottom edge
                    } else {
                        level->setBlock(x, y, (uint8_t)(1 + 8 * 16 + to));   // Top edge
                    }
                }
            } else if (b[0][0] == b[0][1] && b[1][0] == b[1][1]) {
                if (b[0][0]) {
                    level->setBlock(x, y, (uint8_t)(2 + 9 * 16 + to));  // Right edge
                } else {
                    level->setBlock(x, y, (uint8_t)(0 + 9 * 16 + to));  // Left edge
                }
            } else if (b[0][0] == b[1][1] && b[0][1] == b[1][0]) {
                level->setBlock(x, y, (uint8_t)(1 + 9 * 16 + to));  // Center fill (diagonal)
            } else if (b[0][0] == b[1][0]) {
                if (b[0][0]) {
                    if (b[0][1]) {
                        level->setBlock(x, y, (uint8_t)(3 + 10 * 16 + to));  // Inside corner bottom-right
                    } else {
                        level->setBlock(x, y, (uint8_t)(3 + 11 * 16 + to));  // Outside corner
                    }
                } else {
                    if (b[0][1]) {
                        level->setBlock(x, y, (uint8_t)(2 + 8 * 16 + to));   // Top-right corner
                    } else {
                        level->setBlock(x, y, (uint8_t)(0 + 8 * 16 + to));   // Top-left corner
                    }
                }
            } else if (b[0][1] == b[1][1]) {
                if (b[0][1]) {
                    if (b[0][0]) {
                        level->setBlock(x, y, (uint8_t)(3 + 9 * 16 + to));   // Inside corner
                    } else {
                        level->setBlock(x, y, (uint8_t)(3 + 8 * 16 + to));   // Outside corner
                    }
                } else {
                    if (b[0][0]) {
                        level->setBlock(x, y, (uint8_t)(2 + 10 * 16 + to));  // Bottom-right corner
                    } else {
                        level->setBlock(x, y, (uint8_t)(0 + 10 * 16 + to));  // Bottom-left corner
                    }
                }
            } else {
                level->setBlock(x, y, (uint8_t)(0 + 1 * 16 + to));  // Fallback
            }
        }
    }
}

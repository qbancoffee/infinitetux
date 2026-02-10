/**
 * @file MapScene.cpp
 * @brief World map scene implementation.
 */
#include "MapScene.h"
#include "Game.h"
#include "Art.h"
#include "Mario.h"
#include "ImprovedNoise.h"
#include <cmath>
#include <algorithm>
#include <utility>

MapScene::MapScene(Game* game, long seed) : seed(seed), random(seed) {
    this->game = game;
}

void MapScene::init() {
    DEBUG_PRINT("init() starting...");
    worldNumber = -1;
    nextWorld();
    DEBUG_PRINT("init() complete");
}

void MapScene::nextWorld() {
    DEBUG_PRINT("nextWorld() starting, worldNumber=%d", worldNumber);
    
    worldNumber++;
    
    if (worldNumber == 8) {
        game->win();
        return;
    }
    
    moveTime = 0;
    levelId = 0;
    farthest = 0;
    xFarthestCap = 0;
    yFarthestCap = 0;
    
    seed = random.nextLong();
    random = Random(seed);
    
    DEBUG_PRINT("Generating level with seed=%ld", seed);
    
    // Keep generating until we get a valid map (with limit to prevent infinite loop)
    int attempts = 0;
    while (!generateLevel() && attempts < 1000) {
        seed = random.nextLong();
        random = Random(seed);
        attempts++;
        if (attempts % 100 == 0) {
            DEBUG_PRINT("generateLevel attempt %d", attempts);
        }
    }
    
    DEBUG_PRINT("Level generated after %d attempts", attempts);
    
    // If still no valid map, create a simple fallback
    if (attempts >= 1000) {
        // Just create a minimal map
        int width = 320 / 16 + 1;
        int height = 240 / 16 + 1;
        level.clear();
        data.clear();
        level.resize(width, std::vector<int>(height, TILE_GRASS));
        data.resize(width, std::vector<int>(height, 0));
        
        // Place start and end
        level[2][8] = TILE_LEVEL;
        data[2][8] = -11;
        xMario = 2 * 16;
        yMario = 8 * 16;
        
        level[18][8] = TILE_LEVEL;
        data[18][8] = -2;
        xFarthestCap = 18;
        yFarthestCap = 8;
        
        // Connect with road
        for (int x = 3; x < 18; x++) {
            level[x][8] = TILE_ROAD;
            data[x][8] = 0;
        }
        
        // Add a level in between
        level[10][8] = TILE_LEVEL;
        data[10][8] = 1;
    }
}

void MapScene::startMusic() {
    DEBUG_PRINT("startMusic() called");
    Art::startMusic(MUSIC_MAP);
    DEBUG_PRINT("startMusic() complete");
}

bool MapScene::generateLevel() {
    random = Random(seed);
    DEBUG_PRINT("MapScene::generateLevel() with seed=%ld", (long)seed);
    
    ImprovedNoise n0(random.nextLong());
    ImprovedNoise n1(random.nextLong());
    ImprovedNoise dec(random.nextLong());
    
    int width = 320 / 16 + 1;  // 21
    int height = 240 / 16 + 1; // 16
    
    // Initialize arrays
    level.clear();
    data.clear();
    level.resize(width, std::vector<int>(height, TILE_GRASS));
    data.resize(width, std::vector<int>(height, 0));
    
    double xo0 = random.nextDouble() * 512;
    double yo0 = random.nextDouble() * 512;
    double xo1 = random.nextDouble() * 512;
    double yo1 = random.nextDouble() * 512;
    DEBUG_PRINT("  Noise offsets: xo0=%.2f yo0=%.2f xo1=%.2f yo1=%.2f", xo0, yo0, xo1, yo1);
    
    // Generate terrain using Perlin noise - exactly like Java
    int waterCount = 0, grassCount = 0;
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            double xd = ((x + 1) / (double)width - 0.5) * 2;
            double yd = ((y + 1) / (double)height - 0.5) * 2;
            double d = sqrt(xd * xd + yd * yd) * 2;
            if (x == 0 || y == 0 || x >= width - 3 || y >= height - 3) d = 100;
            double t0 = n0.perlinNoise(x * 10.0 + xo0, y * 10.0 + yo0);
            double t1 = n1.perlinNoise(x * 10.0 + xo1, y * 10.0 + yo1);
            double td = t0 - t1;
            double t = td * 2;
            level[x][y] = t > 0 ? TILE_WATER : TILE_GRASS;
            if (level[x][y] == TILE_WATER) waterCount++;
            else grassCount++;
        }
    }
    DEBUG_PRINT("  Terrain: %d water, %d grass (%.1f%% water)", waterCount, grassCount, 
                100.0 * waterCount / (waterCount + grassCount));
    
    // Place levels on grass tiles
    int lowestX = 9999;
    int lowestY = 9999;
    int t = 0;
    for (int i = 0; i < 100 && t < 12; i++) {
        int x = random.nextInt((width - 1) / 3) * 3 + 2;
        int y = random.nextInt((height - 1) / 3) * 3 + 1;
        if (level[x][y] == TILE_GRASS) {
            if (x < lowestX) {
                lowestX = x;
                lowestY = y;
            }
            level[x][y] = TILE_LEVEL;
            data[x][y] = -1;
            t++;
            DEBUG_PRINT("  Placed level %d at (%d, %d)", t, x, y);
        }
    }
    DEBUG_PRINT("  Total levels placed: %d", t);
    
    if (t < 2) {
        DEBUG_PRINT("  FAILED: Not enough levels (need at least 2)");
        return false;
    }
    
    data[lowestX][lowestY] = -2;
    DEBUG_PRINT("  Starting level at (%d, %d)", lowestX, lowestY);
    
    // Connect levels
    while (findConnection(width, height))
        ;
    
    findCaps(width, height);
    DEBUG_PRINT("  After findCaps: xMario=%d yMario=%d xFarthestCap=%d yFarthestCap=%d", 
                xMario, yMario, xFarthestCap, yFarthestCap);
    
    if (xFarthestCap == 0) {
        DEBUG_PRINT("  FAILED: No farthest cap found");
        return false;
    }
    
    data[xFarthestCap][yFarthestCap] = -2;
    DEBUG_PRINT("  Castle at (%d, %d)", xFarthestCap, yFarthestCap);
    
    // Add decorations
    int decorCount = 0;
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            if (level[x][y] == TILE_GRASS && (x != xFarthestCap || y != yFarthestCap - 1)) {
                double t0 = dec.perlinNoise(x * 10.0 + xo0, y * 10.0 + yo0);
                if (t0 > 0) {
                    level[x][y] = TILE_DECORATION;
                    decorCount++;
                }
            }
        }
    }
    DEBUG_PRINT("  Decorations added: %d", decorCount);
    
    // Print final level data
    DEBUG_PRINT("  Final level data:");
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            if (level[x][y] == TILE_LEVEL) {
                const char* typeStr = "unknown";
                if (data[x][y] == 0) typeStr = "completed/start";
                else if (data[x][y] == -1) typeStr = "cap";
                else if (data[x][y] == -2) typeStr = "castle";
                else if (data[x][y] == -3) typeStr = "bonus";
                else if (data[x][y] > 0) typeStr = "numbered";
                DEBUG_PRINT("    Level at (%d,%d) data=%d [%s]", x, y, data[x][y], typeStr);
            }
        }
    }
    
    return true;
}
bool MapScene::findConnection(int width, int height) {
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            if (level[x][y] == TILE_LEVEL && data[x][y] == -1) {
                DEBUG_PRINT("  findConnection: Found unconnected level at (%d,%d)", x, y);
                // Try to connect to adjacent level
                connect(x, y, width, height);
                return true;
            }
        }
    }
    DEBUG_PRINT("  findConnection: No more unconnected levels");
    return false;
}

void MapScene::connect(int xSource, int ySource, int width, int height) {
    int maxDist = 10000;
    int xTarget = 0;
    int yTarget = 0;
    
    DEBUG_PRINT("  connect: Looking for targets from (%d,%d)...", xSource, ySource);
    
    // Find closest level to connect to
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            if (level[x][y] == TILE_LEVEL && data[x][y] == -2) {
                int xd = std::abs(xSource - x);
                int yd = std::abs(ySource - y);
                int d = xd * xd + yd * yd;
                DEBUG_PRINT("    Candidate target: (%d,%d) dist=%d", x, y, d);
                if (d < maxDist) {
                    maxDist = d;
                    xTarget = x;
                    yTarget = y;
                }
            }
        }
    }
    
    DEBUG_PRINT("  connect: (%d,%d) -> (%d,%d) maxDist=%d", xSource, ySource, xTarget, yTarget, maxDist);
    
    // Draw road from source to target (even if no target found, Java does this)
    drawRoad(xSource, ySource, xTarget, yTarget);
    
    // Restore the source tile as a level (drawRoad might have overwritten it)
    level[xSource][ySource] = TILE_LEVEL;
    data[xSource][ySource] = -2;
}

void MapScene::drawRoad(int x0, int y0, int x1, int y1) {
    bool xFirst = random.nextInt(2) == 0;
    
    if (xFirst) {
        while (x0 > x1) {
            data[x0][y0] = 0;
            level[x0--][y0] = TILE_ROAD;
        }
        while (x0 < x1) {
            data[x0][y0] = 0;
            level[x0++][y0] = TILE_ROAD;
        }
    }
    
    while (y0 > y1) {
        data[x0][y0] = 0;
        level[x0][y0--] = TILE_ROAD;
    }
    while (y0 < y1) {
        data[x0][y0] = 0;
        level[x0][y0++] = TILE_ROAD;
    }
    
    if (!xFirst) {
        while (x0 > x1) {
            data[x0][y0] = 0;
            level[x0--][y0] = TILE_ROAD;
        }
        while (x0 < x1) {
            data[x0][y0] = 0;
            level[x0++][y0] = TILE_ROAD;
        }
    }
}

void MapScene::findCaps(int width, int height) {
    int xCap = -1;
    int yCap = -1;
    
    DEBUG_PRINT("  findCaps: scanning for caps...");
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            if (level[x][y] == TILE_LEVEL) {
                int roads = 0;
                for (int xx = x - 1; xx <= x + 1; xx++) {
                    for (int yy = y - 1; yy <= y + 1; yy++) {
                        if (xx >= 0 && xx < width && yy >= 0 && yy < height) {
                            if (level[xx][yy] == TILE_ROAD) roads++;
                        }
                    }
                }
                
                DEBUG_PRINT("    Level at (%d,%d) has %d adjacent roads", x, y, roads);
                
                if (roads == 1) {
                    if (xCap == -1) {
                        xCap = x;
                        yCap = y;
                        DEBUG_PRINT("      -> First cap found!");
                    }
                    data[x][y] = 0;
                } else {
                    data[x][y] = 1;
                }
            }
        }
    }
    
    DEBUG_PRINT("  findCaps: xCap=%d yCap=%d", xCap, yCap);
    
    if (xCap != -1) {
        xMario = xCap * 16;
        yMario = yCap * 16;
        DEBUG_PRINT("  findCaps: Starting travel from (%d,%d)", xCap, yCap);
        travel(xCap, yCap, -1, 0);
    } else {
        DEBUG_PRINT("  findCaps: No cap found! Cannot start travel.");
    }
}

void MapScene::travel(int x, int y, int dir, int depth) {
    // Prevent stack overflow with depth limit
    if (depth > 100) return;
    
    int width = level.size();
    int height = level[0].size();
    
    if (x < 0 || x >= width || y < 0 || y >= height) return;
    
    if (level[x][y] != TILE_ROAD && level[x][y] != TILE_LEVEL) return;
    
    if (level[x][y] == TILE_ROAD) {
        if (data[x][y] == 1) return;
        data[x][y] = 1;
    }
    
    if (level[x][y] == TILE_LEVEL) {
        DEBUG_PRINT("    travel: visiting level at (%d,%d) data=%d depth=%d", x, y, data[x][y], depth);
        if (data[x][y] > 0) {
            if (levelId != 0 && random.nextInt(4) == 0) {
                data[x][y] = -3;  // Bonus level
                DEBUG_PRINT("      -> Bonus level");
            } else {
                data[x][y] = ++levelId;
                DEBUG_PRINT("      -> Level %d", levelId);
            }
        } else if (depth > 0) {
            data[x][y] = -1;
            DEBUG_PRINT("      -> Cap (depth=%d, farthest=%d)", depth, farthest);
            if (depth > farthest) {
                farthest = depth;
                xFarthestCap = x;
                yFarthestCap = y;
                DEBUG_PRINT("      -> NEW farthest cap!");
            }
        } else {
            DEBUG_PRINT("      -> Starting point (depth=0)");
        }
    }
    
    // Java uses depth++ which is post-increment - same value passed to all children
    if (dir != 2) travel(x - 1, y, 0, depth++);
    if (dir != 3) travel(x, y - 1, 1, depth++);
    if (dir != 0) travel(x + 1, y, 2, depth++);
    if (dir != 1) travel(x, y + 1, 3, depth++);
}

void MapScene::tick() {
    xMario += xMarioA;
    yMario += yMarioA;
    tickCount++;
    
    int x = xMario / 16;
    int y = yMario / 16;
    
    if (x >= 0 && x < (int)level.size() && y >= 0 && y < (int)level[0].size()) {
        if (level[x][y] == TILE_ROAD) {
            data[x][y] = 0;  // Mark road as visited
        }
    }
    
    if (moveTime > 0) {
        moveTime--;
    } else {
        xMarioA = 0;
        yMarioA = 0;
        
        // Enter level
        if (canEnterLevel && (keys[Mario::KEY_JUMP] || keys[Mario::KEY_SPEED])) {
            if (x >= 0 && x < (int)level.size() && y >= 0 && y < (int)level[0].size()) {
                DEBUG_PRINT("MapScene: Trying to enter level at (%d,%d) tile=%d data=%d", 
                            x, y, level[x][y], data[x][y]);
                if (level[x][y] == TILE_LEVEL && data[x][y] != -11) {
                    if (data[x][y] != 0 && data[x][y] > -10) {
                        // Build level string (e.g. "1-1", "2-X", "3-?")
                        Mario::levelString = std::to_string(worldNumber + 1) + "-";
                        
                        int difficulty = worldNumber + 1;
                        int type = 0;  // Overworld (TYPE_OVERGROUND)
                        
                        // Use level position to deterministically choose level type
                        Random levelRng(seed + x * 313211 + y * 534321);
                        
                        // Match Java logic exactly:
                        // - For numbered levels (data > 1), 33% chance of underground
                        // - For any negative data (caps, castles, bonus), always castle type
                        if (data[x][y] > 1 && levelRng.nextInt(3) == 0) {
                            type = 1;  // Underground
                        }
                        
                        if (data[x][y] < 0) {
                            // All negative values become castle
                            if (data[x][y] == -2) {
                                // Final castle
                                Mario::levelString += "X";
                                difficulty += 2;
                            } else if (data[x][y] == -1) {
                                // Cap level
                                Mario::levelString += "?";
                            } else {
                                // Bonus level (-3)
                                Mario::levelString += "#";
                                difficulty += 1;
                            }
                            type = 2;  // Castle
                        } else {
                            Mario::levelString += std::to_string(data[x][y]);
                        }
                        
                        DEBUG_PRINT("MapScene: Entering level at (%d,%d) type=%d difficulty=%d", x, y, type, difficulty);
                        Art::stopMusic();
                        game->startLevel(seed * x * y + x * 31871 + y * 21871, difficulty, type);
                    } else {
                        DEBUG_PRINT("MapScene: Cannot enter - data=%d (need !=0 and >-10)", data[x][y]);
                    }
                }
            }
        }
        
        canEnterLevel = !keys[Mario::KEY_JUMP] && !keys[Mario::KEY_SPEED];
        
        if (keys[Mario::KEY_LEFT]) tryWalking(-1, 0);
        if (keys[Mario::KEY_RIGHT]) tryWalking(1, 0);
        if (keys[Mario::KEY_UP]) tryWalking(0, -1);
        if (keys[Mario::KEY_DOWN]) tryWalking(0, 1);
    }
}

/**
 * Attempts to move Mario in the specified direction on the world map.
 * 
 * Movement rules:
 * - Can walk on roads (TILE_ROAD) and levels (TILE_LEVEL)
 * - Roads have a data value that controls access:
 *   - data=0: completed/starting position - can walk from here
 *   - data>0: locked level - must complete previous levels first
 *   - data<-10: special markers (castle entrance, etc.)
 * - In test mode, all level locks are bypassed
 * 
 * @param xd X direction (-1 for left, 1 for right, 0 for none)
 * @param yd Y direction (-1 for up, 1 for down, 0 for none)
 */
void MapScene::tryWalking(int xd, int yd) {
    int x = xMario / 16;
    int y = yMario / 16;
    int xt = x + xd;
    int yt = y + yd;
    
    if (xt < 0 || xt >= (int)level.size() || yt < 0 || yt >= (int)level[0].size()) {
        DEBUG_PRINT("MapScene::tryWalking blocked: target (%d,%d) out of bounds", xt, yt);
        return;
    }
    
    if (level[xt][yt] == TILE_ROAD || level[xt][yt] == TILE_LEVEL) {
        if (level[xt][yt] == TILE_ROAD) {
            // In test mode, bypass all level locks
            if (!g_testMode) {
                // Can only walk on unvisited roads or from visited positions
                if (data[xt][yt] != 0 && data[x][y] != 0 && data[x][y] > -10) {
                    DEBUG_PRINT("MapScene::tryWalking blocked: road at (%d,%d) data=%d, current (%d,%d) data=%d",
                                xt, yt, data[xt][yt], x, y, data[x][y]);
                    return;
                }
            }
        }
        DEBUG_PRINT("MapScene::tryWalking: moving from (%d,%d) to (%d,%d) tile=%d", x, y, xt, yt, level[xt][yt]);
        xMarioA = xd * 8;
        yMarioA = yd * 8;
        int dist = calcDistance(x, y, xd, yd);
        moveTime = dist * 2 + 1;
        DEBUG_PRINT("MapScene::tryWalking: calcDistance=%d, moveTime=%d", dist, moveTime);
    } else {
        DEBUG_PRINT("MapScene::tryWalking blocked: tile at (%d,%d) is %d (not road/level)", xt, yt, level[xt][yt]);
    }
}

/**
 * Calculates how many tiles Mario should walk before stopping.
 * 
 * Mario continues walking until:
 * 1. The next tile is not a road (e.g., hits a level, grass, or water)
 * 2. There's a branch in the road (perpendicular road tile)
 * 
 * This allows Mario to smoothly walk between levels without stopping
 * at every road tile, while still stopping at intersections where
 * the player needs to choose a direction.
 * 
 * @param x Starting X tile position
 * @param y Starting Y tile position  
 * @param xa X direction (-1, 0, or 1)
 * @param ya Y direction (-1, 0, or 1)
 * @return Number of tiles Mario can walk before needing to stop
 */
int MapScene::calcDistance(int x, int y, int xa, int ya) {
    int distance = 0;
    while (true) {
        x += xa;
        y += ya;
        
        // Bounds check - stop at map edges
        if (x < 0 || x >= (int)level.size() || y < 0 || y >= (int)level[0].size()) {
            DEBUG_PRINT("calcDistance: hit bounds at (%d,%d), distance=%d", x, y, distance);
            return distance;
        }
        
        // Stop if not a road (level or other tile type)
        if (level[x][y] != TILE_ROAD) {
            DEBUG_PRINT("calcDistance: hit non-road tile %d at (%d,%d), distance=%d", level[x][y], x, y, distance);
            return distance;
        }
        
        // Check for branches (perpendicular roads or levels)
        // If there's a road/level to the side, this is an intersection - stop here
        int perpX1 = x - ya;  // Perpendicular direction 1
        int perpY1 = y + xa;
        int perpX2 = x + ya;  // Perpendicular direction 2
        int perpY2 = y - xa;
        
        bool hasBranch = false;
        if (perpX1 >= 0 && perpX1 < (int)level.size() && perpY1 >= 0 && perpY1 < (int)level[0].size()) {
            if (level[perpX1][perpY1] == TILE_ROAD || level[perpX1][perpY1] == TILE_LEVEL) {
                hasBranch = true;
            }
        }
        if (perpX2 >= 0 && perpX2 < (int)level.size() && perpY2 >= 0 && perpY2 < (int)level[0].size()) {
            if (level[perpX2][perpY2] == TILE_ROAD || level[perpX2][perpY2] == TILE_LEVEL) {
                hasBranch = true;
            }
        }
        
        if (hasBranch) {
            DEBUG_PRINT("calcDistance: hit branch at (%d,%d), distance=%d", x, y, distance);
            return distance;
        }
        
        distance++;
    }
}

void MapScene::levelWon() {
    int x = xMario / 16;
    int y = yMario / 16;
    
    if (x >= 0 && x < (int)data.size() && y >= 0 && y < (int)data[0].size()) {
        if (data[x][y] == -2) {
            nextWorld();
            return;
        }
        if (data[x][y] != -3) {
            data[x][y] = 0;
        } else {
            data[x][y] = -10;
        }
    }
}

void MapScene::render(SDL_Renderer* renderer, float alpha) {
    static bool debugPrinted = false;
    
    // Draw background color based on world number
    SDL_SetRenderDrawColor(renderer, 80, 160, 80, 255);
    SDL_RenderClear(renderer);
    
    int width = level.size();
    int height = (width > 0) ? level[0].size() : 0;
    
    bool hasMapTiles = !Art::map.empty() && !Art::map[0].empty();
    
    if (!debugPrinted) {
        DEBUG_PRINT("MapScene::render() - Art::map size: %zu columns", Art::map.size());
        if (!Art::map.empty()) {
            DEBUG_PRINT("  Art::map[0] size: %zu rows", Art::map[0].size());
        }
        DEBUG_PRINT("  hasMapTiles: %s", hasMapTiles ? "true" : "false");
        DEBUG_PRINT("  level grid: %d x %d", width, height);
        DEBUG_PRINT("  worldNumber: %d", worldNumber);
        DEBUG_PRINT("  xMario: %d, yMario: %d", xMario, yMario);
        
        // Count tile types
        int grassCount = 0, waterCount = 0, levelCount = 0, roadCount = 0, decorCount = 0;
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                switch (level[x][y]) {
                    case TILE_GRASS: grassCount++; break;
                    case TILE_WATER: waterCount++; break;
                    case TILE_LEVEL: levelCount++; break;
                    case TILE_ROAD: roadCount++; break;
                    case TILE_DECORATION: decorCount++; break;
                }
            }
        }
        DEBUG_PRINT("  Tile counts: grass=%d water=%d level=%d road=%d decor=%d", 
                    grassCount, waterCount, levelCount, roadCount, decorCount);
        
        // Print visual map for debugging
        DEBUG_PRINT("  Visual map (G=grass, W=water, L=level, R=road, D=decor):");
        for (int y = 0; y < height && y < 16; y++) {
            char row[22];
            for (int x = 0; x < width && x < 21; x++) {
                switch (level[x][y]) {
                    case TILE_GRASS: row[x] = 'G'; break;
                    case TILE_WATER: row[x] = 'W'; break;
                    case TILE_LEVEL: row[x] = 'L'; break;
                    case TILE_ROAD: row[x] = 'R'; break;
                    case TILE_DECORATION: row[x] = 'D'; break;
                    default: row[x] = '?'; break;
                }
            }
            row[21] = '\0';
            DEBUG_PRINT("    %2d: %s", y, row);
        }
        
        debugPrinted = true;
    }
    
    // PASS 1: Draw all grass backgrounds first
    for (int x = 0; x < width && x < 320/16; x++) {
        for (int y = 0; y < height && y < 240/16; y++) {
            int screenX = x * 16;
            int screenY = y * 16;
            
            if (hasMapTiles) {
                int bgTile = worldNumber / 4;
                if (bgTile < (int)Art::map.size() && 0 < (int)Art::map[bgTile].size()) {
                    SDL_Rect dst = {screenX, screenY, 16, 16};
                    if (Art::map[bgTile][0]) {
                        SDL_RenderCopy(renderer, Art::map[bgTile][0], nullptr, &dst);
                    }
                }
            }
        }
    }
    
    // PASS 2: Draw all water tiles (edges will overlap onto adjacent grass)
    for (int x = 0; x < width && x < 320/16; x++) {
        for (int y = 0; y < height && y < 240/16; y++) {
            if (level[x][y] != TILE_WATER) continue;
            
            int screenX = x * 16;
            int screenY = y * 16;
            
            if (hasMapTiles) {
                for (int xx = 0; xx < 2; xx++) {
                    for (int yy = 0; yy < 2; yy++) {
                        int p0 = isWater(x * 2 + (xx - 1), y * 2 + (yy - 1)) ? 0 : 1;
                        int p1 = isWater(x * 2 + (xx + 0), y * 2 + (yy - 1)) ? 0 : 1;
                        int p2 = isWater(x * 2 + (xx - 1), y * 2 + (yy + 0)) ? 0 : 1;
                        int p3 = isWater(x * 2 + (xx + 0), y * 2 + (yy + 0)) ? 0 : 1;
                        int s = p0 + p1 * 2 + p2 * 4 + p3 * 8 - 1;
                        
                        if (s >= 0 && s < 14 && s < (int)Art::map.size()) {
                            int tileY = 4 + ((xx + yy) & 1);
                            if (tileY < (int)Art::map[s].size() && Art::map[s][tileY]) {
                                // Draw full 16x16 tile at 8-pixel offset (tiles overlap onto neighbors)
                                SDL_Rect dst = {screenX + xx * 8, screenY + yy * 8, 16, 16};
                                SDL_RenderCopy(renderer, Art::map[s][tileY], nullptr, &dst);
                            }
                        } else if (s == -1) {
                            // Pure water (all corners are water) - use tile 14 (or animated water)
                            // The animated water pass will cover this, but we can draw a base water tile
                            int tileY = 4 + ((xx + yy) & 1);
                            if (14 < (int)Art::map.size() && tileY < (int)Art::map[14].size() && Art::map[14][tileY]) {
                                SDL_Rect dst = {screenX + xx * 8, screenY + yy * 8, 16, 16};
                                SDL_RenderCopy(renderer, Art::map[14][tileY], nullptr, &dst);
                            }
                        }
                    }
                }
            } else {
                SDL_Rect dst = {screenX, screenY, 16, 16};
                SDL_SetRenderDrawColor(renderer, 64, 64, 200, 255);
                SDL_RenderFillRect(renderer, &dst);
            }
        }
    }
    
    // PASS 3: Draw roads and levels on top
    for (int x = 0; x < width && x < 320/16; x++) {
        for (int y = 0; y < height && y < 240/16; y++) {
            int screenX = x * 16;
            int screenY = y * 16;
            int tile = level[x][y];
            int d = data[x][y];
            
            if (tile == TILE_LEVEL) {
                SDL_Rect dst = {screenX, screenY, 16, 16};
                
                if (hasMapTiles) {
                    if (d == 0) {
                        // Completed level
                        if (0 < (int)Art::map.size() && 7 < (int)Art::map[0].size() && Art::map[0][7])
                            SDL_RenderCopy(renderer, Art::map[0][7], nullptr, &dst);
                    } else if (d == -1) {
                        // Uncompleted level
                        if (3 < (int)Art::map.size() && 8 < (int)Art::map[3].size() && Art::map[3][8])
                            SDL_RenderCopy(renderer, Art::map[3][8], nullptr, &dst);
                    } else if (d == -3) {
                        // Bonus level
                        if (0 < (int)Art::map.size() && 8 < (int)Art::map[0].size() && Art::map[0][8])
                            SDL_RenderCopy(renderer, Art::map[0][8], nullptr, &dst);
                    } else if (d == -10) {
                        // Completed bonus
                        if (1 < (int)Art::map.size() && 8 < (int)Art::map[1].size() && Art::map[1][8])
                            SDL_RenderCopy(renderer, Art::map[1][8], nullptr, &dst);
                    } else if (d == -11) {
                        // Start position
                        if (1 < (int)Art::map.size() && 7 < (int)Art::map[1].size() && Art::map[1][7])
                            SDL_RenderCopy(renderer, Art::map[1][7], nullptr, &dst);
                    } else if (d == -2) {
                        // Castle - two tiles high
                        SDL_Rect dstTop = {screenX, screenY - 16, 16, 16};
                        if (2 < (int)Art::map.size() && 7 < (int)Art::map[2].size() && Art::map[2][7])
                            SDL_RenderCopy(renderer, Art::map[2][7], nullptr, &dstTop);
                        if (2 < (int)Art::map.size() && 8 < (int)Art::map[2].size() && Art::map[2][8])
                            SDL_RenderCopy(renderer, Art::map[2][8], nullptr, &dst);
                    } else if (d > 0) {
                        // Numbered level
                        int tileX = d - 1;
                        if (tileX < (int)Art::map.size() && 6 < (int)Art::map[tileX].size() && Art::map[tileX][6])
                            SDL_RenderCopy(renderer, Art::map[tileX][6], nullptr, &dst);
                    }
                } else {
                    // Fallback colored rectangles
                    if (d == -2) {
                        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
                    } else if (d == -11 || d == 0 || d == -10) {
                        SDL_SetRenderDrawColor(renderer, 40, 120, 40, 255);
                    } else if (d == -3) {
                        SDL_SetRenderDrawColor(renderer, 200, 200, 0, 255);
                    } else {
                        SDL_SetRenderDrawColor(renderer, 200, 50, 50, 255);
                    }
                    SDL_RenderFillRect(renderer, &dst);
                }
            } else if (tile == TILE_ROAD) {
                // Road - use tile based on connectivity
                if (hasMapTiles) {
                    int p0 = isRoad(x - 1, y) ? 1 : 0;
                    int p1 = isRoad(x, y - 1) ? 1 : 0;
                    int p2 = isRoad(x + 1, y) ? 1 : 0;
                    int p3 = isRoad(x, y + 1) ? 1 : 0;
                    int s = p0 + p1 * 2 + p2 * 4 + p3 * 8;
                    
                    SDL_Rect dst = {screenX, screenY, 16, 16};
                    if (s < (int)Art::map.size() && 2 < (int)Art::map[s].size() && Art::map[s][2])
                        SDL_RenderCopy(renderer, Art::map[s][2], nullptr, &dst);
                } else {
                    SDL_Rect dst = {screenX, screenY, 16, 16};
                    SDL_SetRenderDrawColor(renderer, 139, 90, 43, 255);
                    SDL_RenderFillRect(renderer, &dst);
                }
            }
            // TILE_GRASS and TILE_DECORATION handled in later passes
        }
    }
    
    // Draw animated water
    if (hasMapTiles) {
        for (int x = 0; x < width && x < 320/16; x++) {
            for (int y = 0; y < height && y < 240/16; y++) {
                if (level[x][y] == TILE_WATER) {
                    if (isWater(x * 2 - 1, y * 2 - 1)) {
                        int frame = (tickCount / 6 + y) % 4;
                        if (15 < (int)Art::map.size() && (4 + frame) < (int)Art::map[15].size() && Art::map[15][4 + frame]) {
                            SDL_Rect dst = {x * 16 - 8, y * 16 - 8, 16, 16};
                            SDL_RenderCopy(renderer, Art::map[15][4 + frame], nullptr, &dst);
                        }
                    }
                }
            }
        }
    }
    
    // Draw decorations
    if (hasMapTiles) {
        for (int x = 0; x < width && x < 320/16; x++) {
            for (int y = 0; y < height && y < 240/16; y++) {
                if (level[x][y] == TILE_DECORATION) {
                    int frame = ((tickCount + y * 12) / 6) % 4;
                    int variant = worldNumber % 4;
                    if (frame < (int)Art::map.size() && (10 + variant) < (int)Art::map[frame].size() && Art::map[frame][10 + variant]) {
                        SDL_Rect dst = {x * 16, y * 16, 16, 16};
                        SDL_RenderCopy(renderer, Art::map[frame][10 + variant], nullptr, &dst);
                    }
                }
            }
        }
    }
    
    // Draw Mario
    int marioScreenX = xMario + (int)(xMarioA * alpha);
    int marioScreenY = yMario + (int)(yMarioA * alpha) - 6;
    
    if (hasMapTiles) {
        int frame = (tickCount / 6) % 2;
        if (!Mario::large) {
            // Small Mario on map
            if (frame < (int)Art::map.size() && 1 < (int)Art::map[frame].size() && Art::map[frame][1]) {
                SDL_Rect dst = {marioScreenX, marioScreenY, 16, 16};
                SDL_RenderCopy(renderer, Art::map[frame][1], nullptr, &dst);
            }
        } else {
            // Large Mario on map (2 tiles high)
            int baseX = Mario::fire ? 4 : 2;
            if ((baseX + frame) < (int)Art::map.size()) {
                if (0 < (int)Art::map[baseX + frame].size() && Art::map[baseX + frame][0]) {
                    SDL_Rect dstTop = {marioScreenX, marioScreenY - 16, 16, 16};
                    SDL_RenderCopy(renderer, Art::map[baseX + frame][0], nullptr, &dstTop);
                }
                if (1 < (int)Art::map[baseX + frame].size() && Art::map[baseX + frame][1]) {
                    SDL_Rect dst = {marioScreenX, marioScreenY, 16, 16};
                    SDL_RenderCopy(renderer, Art::map[baseX + frame][1], nullptr, &dst);
                }
            }
        }
    } else {
        SDL_Rect dst = {marioScreenX, marioScreenY, 16, 16};
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &dst);
    }
    
    // Draw HUD
    char buf[32];
    snprintf(buf, sizeof(buf), "TUX %02d", Mario::lives);
    drawStringDropShadow(buf, 0, 0, 7);
    
    snprintf(buf, sizeof(buf), "WORLD %d", worldNumber + 1);
    drawStringDropShadow(buf, 32, 0, 7);
}

void MapScene::drawStringDropShadow(const std::string& text, int x, int y, int c) {
    Art::drawString(text, x * 8 + 5, y * 8 + 5, 0);  // Shadow
    Art::drawString(text, x * 8 + 4, y * 8 + 4, c);  // Main text
}

bool MapScene::isRoad(int x, int y) {
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x >= (int)level.size()) return false;
    if (y >= (int)level[0].size()) return false;
    return level[x][y] == TILE_ROAD || level[x][y] == TILE_LEVEL;
}

bool MapScene::isWater(int x, int y) {
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    
    // Check 2x2 area
    for (int xx = 0; xx < 2; xx++) {
        for (int yy = 0; yy < 2; yy++) {
            int tx = (x + xx) / 2;
            int ty = (y + yy) / 2;
            if (tx >= (int)level.size() || ty >= (int)level[0].size()) return false;
            if (level[tx][ty] != TILE_WATER) return false;
        }
    }
    return true;
}

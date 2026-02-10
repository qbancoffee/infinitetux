/**
 * BgRenderer - Renders parallax scrolling background layers
 * 
 * Creates a procedurally generated background level and renders it
 * with parallax scrolling based on camera position. Different level
 * types (overground, underground, castle) have different visual styles.
 * 
 * The background level is generated once at construction and is large
 * enough to handle extended scrolling (2048+ tiles wide).
 */

#include "BgRenderer.h"
#include "Art.h"
#include "Level.h"
#include <random>

BgRenderer::BgRenderer(int width, int height, int levelType, int distance, bool distant)
    : width(width), height(height), levelType(levelType), distance(distance) {
    // Generate a large background level (2048 tiles like Java)
    // This ensures we don't run out of level during long scrolling
    int bgWidth = 2048;
    int bgHeight = 15;
    bgLevel = generateBgLevel(bgWidth, bgHeight, distant, levelType);
}

BgRenderer::~BgRenderer() {
    if (bgLevel) delete bgLevel;
}

void BgRenderer::setCam(int newXCam, int newYCam) {
    // Apply distance factor (parallax)
    xCam = newXCam / distance;
    yCam = newYCam / distance;
}

Level* BgRenderer::generateBgLevel(int w, int h, bool distant, int type) {
    Level* level = new Level(w, h);
    std::mt19937 random(std::random_device{}());
    
    if (type == 0) {  // TYPE_OVERGROUND
        int range = distant ? 4 : 6;
        int offs = distant ? 2 : 1;
        std::uniform_int_distribution<> dist(0, range - 1);
        int oh = dist(random) + offs;
        int hVal = dist(random) + offs;
        
        for (int x = 0; x < w; x++) {
            oh = hVal;
            while (oh == hVal) {
                hVal = dist(random) + offs;
            }
            for (int y = 0; y < h; y++) {
                int h0 = (oh < hVal) ? oh : hVal;
                int h1 = (oh < hVal) ? hVal : oh;
                if (y < h0) {
                    if (distant) {
                        int s = 2;
                        if (y < 2) s = y;
                        level->setBlock(x, y, (uint8_t)(4 + s * 8));
                    } else {
                        level->setBlock(x, y, (uint8_t)5);
                    }
                } else if (y == h0) {
                    int s = (h0 == hVal) ? 0 : 1;
                    s += distant ? 2 : 0;
                    level->setBlock(x, y, (uint8_t)s);
                } else if (y == h1) {
                    int s = (h0 == hVal) ? 0 : 1;
                    s += distant ? 2 : 0;
                    level->setBlock(x, y, (uint8_t)(s + 16));
                } else {
                    int s = (y > h1) ? 1 : 0;
                    if (h0 == oh) s = 1 - s;
                    s += distant ? 2 : 0;
                    level->setBlock(x, y, (uint8_t)(s + 8));
                }
            }
        }
    } else if (type == 1) {  // TYPE_UNDERGROUND
        if (distant) {
            int tt = 0;
            std::uniform_real_distribution<> realDist(0.0, 1.0);
            for (int x = 0; x < w; x++) {
                if (realDist(random) < 0.75) tt = 1 - tt;
                for (int y = 0; y < h; y++) {
                    int t = tt;
                    int yy = y - 2;
                    if (yy < 0 || yy > 4) {
                        yy = 2;
                        t = 0;
                    }
                    level->setBlock(x, y, (uint8_t)(4 + t + (3 + yy) * 8));
                }
            }
        } else {
            for (int x = 0; x < w; x++) {
                for (int y = 0; y < h; y++) {
                    int t = x % 2;
                    int yy = y - 1;
                    if (yy < 0 || yy > 7) {
                        yy = 7;
                        t = 0;
                    }
                    if (t == 0 && yy > 1 && yy < 5) {
                        t = -1;
                        yy = 0;
                    }
                    level->setBlock(x, y, (uint8_t)(6 + t + yy * 8));
                }
            }
        }
    } else {  // TYPE_CASTLE
        if (distant) {
            for (int x = 0; x < w; x++) {
                for (int y = 0; y < h; y++) {
                    int t = x % 2;
                    int yy = y - 1;
                    if (yy > 2 && yy < 5) {
                        yy = 2;
                    } else if (yy >= 5) {
                        yy -= 2;
                    }
                    if (yy < 0) {
                        t = 0;
                        yy = 5;
                    } else if (yy > 4) {
                        t = 1;
                        yy = 5;
                    } else if (t < 1 && yy == 3) {
                        t = 0;
                        yy = 3;
                    } else if (t < 1 && yy > 0 && yy < 3) {
                        t = 0;
                        yy = 2;
                    }
                    level->setBlock(x, y, (uint8_t)(1 + t + (yy + 4) * 8));
                }
            }
        } else {
            for (int x = 0; x < w; x++) {
                for (int y = 0; y < h; y++) {
                    int t = x % 3;
                    int yy = y - 1;
                    if (yy > 2 && yy < 5) {
                        yy = 2;
                    } else if (yy >= 5) {
                        yy -= 2;
                    }
                    if (yy < 0) {
                        t = 1;
                        yy = 5;
                    } else if (yy > 4) {
                        t = 2;
                        yy = 5;
                    } else if (t < 2 && yy == 4) {
                        t = 2;
                        yy = 4;
                    } else if (t < 2 && yy > 0 && yy < 4) {
                        t = 4;
                        yy = -3;
                    }
                    level->setBlock(x, y, (uint8_t)(1 + t + (yy + 3) * 8));
                }
            }
        }
    }
    
    return level;
}

void BgRenderer::render(SDL_Renderer* renderer, int tick) {
    if (Art::bg.empty() || !bgLevel) return;
    
    // Only draw sky color for the distant (far) layer
    if (distance == 4) {  // Distant layer
        if (levelType == 0) {
            SDL_SetRenderDrawColor(renderer, 92, 148, 252, 255);
        } else if (levelType == 1) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 48, 24, 24, 255);
        }
        
        SDL_Rect bgRect = {0, 0, width, height};
        SDL_RenderFillRect(renderer, &bgRect);
    }
    
    // Draw background tiles
    int xTileStart = xCam / 32;
    int yTileStart = yCam / 32;
    int xTileEnd = (xCam + width) / 32 + 1;
    int yTileEnd = (yCam + height) / 32 + 1;
    
    for (int x = xTileStart; x <= xTileEnd; x++) {
        for (int y = yTileStart; y <= yTileEnd; y++) {
            int b = bgLevel->getBlock(x, y) & 0xff;
            
            // Art.bg[b % 8][b / 8] - column = b % 8, row = b / 8
            int xTile = b % 8;
            int yTile = b / 8;
            
            if (xTile < (int)Art::bg.size() && yTile < (int)Art::bg[xTile].size()) {
                SDL_Rect dst = {x * 32 - xCam, y * 32 - yCam - 16, 32, 32};
                SDL_RenderCopy(renderer, Art::bg[xTile][yTile], nullptr, &dst);
            }
        }
    }
}

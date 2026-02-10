/**
 * @file LevelRenderer.cpp
 * @brief Tile renderer implementation.
 */
#include "LevelRenderer.h"
#include "Level.h"
#include "Art.h"
#include <cmath>

LevelRenderer::LevelRenderer(Level* level, int width, int height)
    : level(level), width(width), height(height) {}

void LevelRenderer::render(SDL_Renderer* renderer, int tick) {
    render(renderer, tick, 1.0f);
}

void LevelRenderer::render(SDL_Renderer* renderer, int tick, float alpha) {
    if (!level || Art::level.empty()) return;
    
    int xTileStart = xCam / 16;
    int yTileStart = yCam / 16;
    int xTileEnd = (xCam + width) / 16 + 1;
    int yTileEnd = (yCam + height) / 16 + 1;
    
    for (int x = xTileStart; x <= xTileEnd; x++) {
        for (int y = yTileStart; y <= yTileEnd; y++) {
            int b = level->getBlock(x, y) & 0xff;
            if (b == 0) continue;
            
            // Calculate bump Y offset
            int yo = 0;
            if (x >= 0 && y >= 0 && x < level->width && y < level->height) {
                int bumpData = level->data[x][y];
                if (bumpData > 0) {
                    yo = (int)(std::sin((bumpData - alpha) / 4.0f * 3.14159f) * 8);
                }
            }
            
            // Tile coordinates: column = b % 16, row = b / 16
            int xTile = b % 16;
            int yTile = b / 16;
            
            // Handle animated tiles
            if ((Level::TILE_BEHAVIORS[b] & Level::BIT_ANIMATED) > 0) {
                int animTime = (tick / 3) % 4;
                
                // Special animation for question blocks (column 0-3, row 1)
                if ((b % 16) / 4 == 0 && b / 16 == 1) {
                    animTime = (tick / 2 + (x + y) / 8) % 20;
                    if (animTime > 3) animTime = 0;
                }
                // Used/empty blocks don't animate
                if ((b % 16) / 4 == 3 && b / 16 == 0) {
                    animTime = 2;
                }
                
                // For animated tiles: column = (b % 16) / 4 * 4 + animTime
                xTile = (b % 16) / 4 * 4 + animTime;
            }
            
            if (xTile < (int)Art::level.size() && yTile < (int)Art::level[xTile].size()) {
                SDL_Rect dst = {x * 16 - xCam, y * 16 - yCam - yo, 16, 16};
                SDL_RenderCopy(renderer, Art::level[xTile][yTile], nullptr, &dst);
            }
        }
    }
}

void LevelRenderer::setLevel(Level* newLevel) {
    level = newLevel;
}

void LevelRenderer::renderStatic(SDL_Renderer* renderer) {
    // Render without animation
}

void LevelRenderer::renderExit0(SDL_Renderer* renderer, int tick, float alpha, bool bar) {
    if (!level || Art::level.empty()) return;
    
    // Draw left pole
    for (int y = level->yExit - 8; y < level->yExit; y++) {
        int tileY = (y == level->yExit - 8) ? 4 : 5;
        if (12 < (int)Art::level.size() && tileY < (int)Art::level[12].size() && Art::level[12][tileY]) {
            SDL_Rect dst = {(level->xExit << 4) - xCam - 16, (y << 4) - yCam, 16, 16};
            SDL_RenderCopy(renderer, Art::level[12][tileY], nullptr, &dst);
        }
    }
    
    // Draw bouncing bar if level not won
    if (bar) {
        int yh = level->yExit * 16 - (int)((std::sin((tick + alpha) / 20.0f) * 0.5f + 0.5f) * 7 * 16) - 8;
        
        // Left side of bar
        if (12 < (int)Art::level.size() && 3 < (int)Art::level[12].size() && Art::level[12][3]) {
            SDL_Rect dst = {(level->xExit << 4) - xCam - 16, yh - yCam, 16, 16};
            SDL_RenderCopy(renderer, Art::level[12][3], nullptr, &dst);
        }
        // Right side of bar  
        if (13 < (int)Art::level.size() && 3 < (int)Art::level[13].size() && Art::level[13][3]) {
            SDL_Rect dst = {(level->xExit << 4) - xCam, yh - yCam, 16, 16};
            SDL_RenderCopy(renderer, Art::level[13][3], nullptr, &dst);
        }
    }
}

void LevelRenderer::renderExit1(SDL_Renderer* renderer, int tick, float alpha) {
    if (!level || Art::level.empty()) return;
    
    // Draw right pole
    for (int y = level->yExit - 8; y < level->yExit; y++) {
        int tileY = (y == level->yExit - 8) ? 4 : 5;
        if (13 < (int)Art::level.size() && tileY < (int)Art::level[13].size() && Art::level[13][tileY]) {
            SDL_Rect dst = {(level->xExit << 4) - xCam + 16, (y << 4) - yCam, 16, 16};
            SDL_RenderCopy(renderer, Art::level[13][tileY], nullptr, &dst);
        }
    }
}

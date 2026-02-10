/**
 * @file Sprite.cpp
 * @brief Base sprite implementation.
 */
#include "Sprite.h"
#include "Art.h"
#include <cmath>

Sprite::Sprite() {}

void Sprite::move() {
    x += xa;
    y += ya;
}

/**
 * Renders the sprite to the screen with interpolation for smooth animation.
 * 
 * This method handles:
 * - Position interpolation between xOld/yOld and x/y based on alpha (0.0 to 1.0)
 * - Horizontal flipping via xFlipPic (e.g., when enemy faces left)
 * - Vertical flipping via yFlipPic or negative hPic (used for death animations
 *   where enemies flip upside down when killed by fireballs)
 * 
 * @param renderer The SDL renderer to draw to
 * @param alpha Interpolation factor (0.0 = previous position, 1.0 = current position)
 */
void Sprite::render(SDL_Renderer* renderer, float alpha) {
    if (!visible || !sheet || sheet->empty()) return;
    
    // Interpolate position for smooth rendering between physics ticks
    int xPixel = (int)(xOld + (x - xOld) * alpha) - xPicO;
    int yPixel = (int)(yOld + (y - yOld) * alpha) - yPicO;
    
    if (xPic >= 0 && xPic < (int)sheet->size() && 
        yPic >= 0 && yPic < (int)(*sheet)[xPic].size()) {
        SDL_Texture* tex = (*sheet)[xPic][yPic];
        if (tex) {
            // Handle negative hPic (death animation flips sprite upside down)
            // In Java, negative height causes vertical flip; in SDL we use SDL_FLIP_VERTICAL
            int renderHeight = hPic;
            bool flipVertical = yFlipPic;
            
            if (hPic < 0) {
                renderHeight = -hPic;
                flipVertical = !flipVertical;  // Toggle vertical flip
            }
            
            SDL_Rect dst = {xPixel, yPixel, wPic, renderHeight};
            
            // Combine flip flags
            SDL_RendererFlip flip = SDL_FLIP_NONE;
            if (xFlipPic) flip = (SDL_RendererFlip)(flip | SDL_FLIP_HORIZONTAL);
            if (flipVertical) flip = (SDL_RendererFlip)(flip | SDL_FLIP_VERTICAL);
            
            SDL_RenderCopyEx(renderer, tex, nullptr, &dst, 0, nullptr, flip);
        }
    }
}

void Sprite::tick() {
    xOld = x;
    yOld = y;
    move();
}

void Sprite::tickNoMove() {
    xOld = x;
    yOld = y;
}

float Sprite::getX(float alpha) const {
    return (xOld + (x - xOld) * alpha) - xPicO;
}

float Sprite::getY(float alpha) const {
    return (yOld + (y - yOld) * alpha) - yPicO;
}

void Sprite::collideCheck() {}
void Sprite::bumpCheck(int xTile, int yTile) {}
bool Sprite::shellCollideCheck(Shell* shell) { return false; }
void Sprite::release(Mario* mario) {}
bool Sprite::fireballCollideCheck(Fireball* fireball) { return false; }

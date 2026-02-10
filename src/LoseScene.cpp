/**
 * LoseScene.cpp - Game over screen with animated ghost
 * 
 * Displayed when Mario runs out of lives. Shows an animated ghost
 * sprite and "Game over!" text. Press jump to return to title.
 */

// LoseScene.cpp
#include "LoseScene.h"
#include "Game.h"
#include "Art.h"
#include "Mario.h"

LoseScene::LoseScene(Game* game) {
    this->game = game;
}

void LoseScene::init() {
    Art::stopMusic();
    wasDown = true;  // Prevent immediate trigger if key held
}

void LoseScene::tick() {
    tickCount++;
    
    // Only trigger on key press, not hold
    if (!wasDown && keys[Mario::KEY_JUMP]) {
        game->toTitle();
    }
    if (keys[Mario::KEY_JUMP]) {
        wasDown = false;
    }
}

void LoseScene::render(SDL_Renderer* renderer, float alpha) {
    // Pinkish-brown background (matches Java: #a07070)
    SDL_SetRenderDrawColor(renderer, 160, 112, 112, 255);
    SDL_RenderClear(renderer);
    
    // Draw animated ghost sprite
    // Animation: frames 0,1,2,3,4,5,4,3,2,1 repeating (bounce)
    int f = (tickCount / 3) % 10;
    if (f >= 6) f = 10 - f;  // Bounce back: 6->4, 7->3, 8->2, 9->1
    
    if (!Art::gameOver.empty() && f < (int)Art::gameOver.size() && 
        !Art::gameOver[f].empty()) {
        SDL_Texture* ghostTex = Art::gameOver[f][0];
        if (ghostTex) {
            // Center ghost: 160 - 48 = 112 for x, 100 - 32 = 68 for y
            SDL_Rect dst = {160 - 48, 100 - 32, 96, 64};
            SDL_RenderCopy(renderer, ghostTex, nullptr, &dst);
        }
    }
    
    // Draw "Game over!" text centered
    const char* msg = "Game over!";
    int msgLen = 10;
    Art::drawString(msg, 160 - msgLen * 4, 160, 0);
}

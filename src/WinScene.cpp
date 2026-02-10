/**
 * @file WinScene.cpp
 * @brief Victory scene implementation.
 */
// WinScene.cpp
#include "WinScene.h"
#include "Game.h"
#include "Art.h"
#include "Mario.h"

WinScene::WinScene(Game* game) {
    this->game = game;
}

void WinScene::init() {
    Art::stopMusic();
}

void WinScene::tick() {
    tickCount++;
    
    if (tickCount > 120) {
        if (keys[Mario::KEY_JUMP] || keys[Mario::KEY_SPEED]) {
            game->toTitle();
        }
    }
}

void WinScene::render(SDL_Renderer* renderer, float alpha) {
    // Dark blue background
    SDL_SetRenderDrawColor(renderer, 0, 0, 64, 255);
    SDL_RenderClear(renderer);
    
    // Draw congratulations text
    Art::drawString("CONGRATULATIONS!", SCREEN_WIDTH / 2 - 64, 60, 0);
    Art::drawString("YOU SAVED THE", SCREEN_WIDTH / 2 - 52, 100, 0);
    Art::drawString("MUSHROOM KINGDOM!", SCREEN_WIDTH / 2 - 68, 120, 0);
    
    Art::drawString("PRESS S TO CONTINUE", SCREEN_WIDTH / 2 - 76, 180, 0);
}

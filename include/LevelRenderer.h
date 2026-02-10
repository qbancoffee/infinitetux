/**
 * @file LevelRenderer.h
 * @brief Tile map renderer.
 * @ingroup level
 * 
 * LevelRenderer draws visible tiles with camera scrolling,
 * tile animation, and bump effects.
 */
#pragma once
#include "Common.h"

class Level;

class LevelRenderer {
public:
    int xCam = 0;
    int yCam = 0;
    
    LevelRenderer(Level* level, int width, int height);
    void render(SDL_Renderer* renderer, int tick);
    void render(SDL_Renderer* renderer, int tick, float alpha);
    void setLevel(Level* level);
    void renderStatic(SDL_Renderer* renderer);
    void renderExit0(SDL_Renderer* renderer, int tick, float alpha, bool bar);
    void renderExit1(SDL_Renderer* renderer, int tick, float alpha);

private:
    Level* level;
    int width, height;
};

/**
 * @file BgRenderer.h
 * @brief Parallax background renderer.
 * @ingroup level
 * 
 * BgRenderer draws scrolling backgrounds with parallax
 * effect for depth perception.
 */
#pragma once
#include "Common.h"

class Level;

class BgRenderer {
public:
    int xCam = 0;
    int yCam = 0;
    
    BgRenderer(int width, int height, int levelType, int distance, bool distant);
    ~BgRenderer();
    void setCam(int xCam, int yCam);
    void render(SDL_Renderer* renderer, int tick);

private:
    int width, height;
    int levelType;
    int distance;
    Level* bgLevel = nullptr;
    
    Level* generateBgLevel(int w, int h, bool distant, int type);
};

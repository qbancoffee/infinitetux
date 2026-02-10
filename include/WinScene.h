/**
 * @file WinScene.h
 * @brief Victory scene.
 * @ingroup scenes
 * 
 * WinScene shows congratulations when all worlds
 * are completed.
 */
#pragma once
#include "Scene.h"

class WinScene : public Scene {
public:
    WinScene(Game* game);
    void init() override;
    void tick() override;
    void render(SDL_Renderer* renderer, float alpha) override;

private:
    int tickCount = 0;
};

/**
 * @file LoseScene.h
 * @brief Game over scene.
 * @ingroup scenes
 * 
 * LoseScene shows game over when all lives are lost.
 */
#pragma once
#include "Scene.h"

class LoseScene : public Scene {
public:
    LoseScene(Game* game);
    void init() override;
    void tick() override;
    void render(SDL_Renderer* renderer, float alpha) override;

private:
    int tickCount = 0;
    bool wasDown = true;  ///< Prevents immediate trigger if key held from previous scene
};

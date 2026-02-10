/**
 * @file Scene.h
 * @brief Abstract base class for game scenes.
 * @ingroup scenes
 * 
 * Scene defines the interface for all game screens.
 * Each scene handles its own update logic and rendering.
 */
#pragma once
#include "Common.h"

class Game;

class Scene {
public:
    static bool keys[16];
    
    Game* game = nullptr;
    
    Scene() = default;
    virtual ~Scene() = default;
    
    virtual void init() {}
    virtual void tick() = 0;
    virtual void render(SDL_Renderer* renderer, float alpha) = 0;
    
    virtual void toggleKey(int key, bool pressed) {
        if (key >= 0 && key < 16) {
            keys[key] = pressed;
        }
    }
    
    /**
     * Handle test mode key presses.
     * Override in LevelScene to handle Mario state changes and enemy spawning.
     * @param key The key character: 'i'/'o'/'p' for Mario state, '0'-'9' for enemies
     */
    virtual void handleTestKey(char key) {}
    
    /**
     * Handle pause key press (Enter).
     * Override in LevelScene to toggle game pause.
     */
    virtual void handlePauseKey() {}
    
    static void resetKeys() {
        for (int i = 0; i < 16; i++) {
            keys[i] = false;
        }
    }
};

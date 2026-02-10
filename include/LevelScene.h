/**
 * @file LevelScene.h
 * @brief Main gameplay scene.
 * @ingroup scenes
 * 
 * LevelScene manages all gameplay: sprites, collision,
 * camera, HUD, and win/lose conditions.
 */
#pragma once
#include "Scene.h"
#include <vector>
#include <memory>

class Level;
class Mario;
class Sprite;
class LevelRenderer;
class BgRenderer;

class LevelScene : public Scene {
public:
    std::vector<Sprite*> sprites;
    std::vector<Sprite*> spritesToAdd;
    std::vector<Sprite*> spritesToRemove;
    
    Level* level = nullptr;
    Mario* mario = nullptr;
    float xCam = 0, yCam = 0, xCamO = 0, yCamO = 0;
    
    bool paused = false;
    bool userPaused = false;  ///< True when user presses Enter to pause
    int startTime = 0;
    int timeLeft = 0;
    int fireballsOnScreen = 0;
    
    LevelScene(Game* game, long seed, int levelDifficulty, int type);
    ~LevelScene();
    
    void init() override;
    void tick() override;
    void render(SDL_Renderer* renderer, float alpha) override;
    void handleTestKey(char key) override;
    void handlePauseKey() override;
    
    void addSprite(Sprite* sprite);
    void removeSprite(Sprite* sprite);
    
    void bump(int x, int y, bool canBreakBricks);
    void bumpInto(int x, int y);
    void checkShellCollide(Sprite* shell);
    void checkFireballCollide(Sprite* fireball);
    void convertEnemiesToCoins();  // Convert all enemies to coins when level is won

private:
    LevelRenderer* layer = nullptr;
    BgRenderer* bgLayer[2] = {nullptr, nullptr};
    
    int tickCount = 0;
    bool isFast = false;
    bool isMusicStopped = false;
    
    long levelSeed;
    int levelType;
    int musicType;
    int levelDifficulty;
    
    // Iris wipe (blackout) rendering - matches Java implementation
    void renderBlackout(SDL_Renderer* renderer, int x, int y, int radius);
};

/**
 * @file Game.h
 * @brief Main game controller and loop.
 * @ingroup core
 * 
 * Game manages:
 * - SDL initialization and cleanup
 * - Main game loop (events, tick, render)
 * - Scene transitions
 * - Display settings (fullscreen, scaling)
 */
#pragma once
#include "Common.h"
#include <memory>

class Scene;
class MapScene;

// Pending scene change types
enum class PendingScene {
    NONE,
    TITLE,
    OPTIONS,
    START_GAME,
    WIN,
    LOSE,
    LEVEL,        // Start a new level (uses pendingLevel* fields)
    LEVEL_FAILED, // Return to map after failing level
    LEVEL_WON     // Return to map after winning level
};

class Game {
public:
    Game();
    ~Game();
    
    bool init(bool useDefaultBindings = false);
    void run();
    void cleanup();
    
    // Scene management (these now queue scene changes for end of frame)
    void startLevel(long seed, int difficulty, int type);
    void levelFailed();
    void levelWon();
    void win();
    void lose();
    void toTitle();
    void toOptions();
    void startGame();
    
    // Display management
    void setFullscreen(bool fullscreen);
    void toggleFullscreen();
    void cycleScaleQuality();
    bool isFullscreen() const { return fullscreenMode; }
    
    SDL_Window* getWindow() const { return window; }
    SDL_Renderer* getRenderer() const { return renderer; }

private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* screenTexture = nullptr;
    
    bool running = false;
    bool useScale2x = false;
    bool fullscreenMode = false;
    int scaleQuality = 0;  // 0=nearest, 1=linear, 2=best
    int delay = 0;
    
    Scene* scene = nullptr;
    MapScene* mapScene = nullptr;
    
    // Pending scene change (processed at end of frame to avoid use-after-free)
    PendingScene pendingScene = PendingScene::NONE;
    long pendingLevelSeed = 0;
    int pendingLevelDifficulty = 0;
    int pendingLevelType = 0;
    
    void handleEvents();
    void updateGameInput();
    void adjustFPS();
    void updateViewport();
    void processPendingSceneChange();
    void doSceneChange(PendingScene sceneType);
};

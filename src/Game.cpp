/**
 * @file Game.cpp
 * @brief Main game loop implementation.
 */
#include "Game.h"
#include "Art.h"
#include "Scene.h"
#include "TitleScene.h"
#include "MapScene.h"
#include "LevelScene.h"
#include "WinScene.h"
#include "LoseScene.h"
#include "OptionsScene.h"
#include "Mario.h"
#include "Level.h"
#include "InputConfig.h"
#include <iostream>
#include <cstdio>
#include <fstream>

#ifndef INFINITE_TUX_DATADIR
#define INFINITE_TUX_DATADIR ""
#endif

// Helper function to check if a directory exists and contains resources
static bool checkResourcePath(const std::string& path) {
    std::string testFile = path + "mariosheet.png";
    std::ifstream f(testFile);
    return f.good();
}

// Find the resources directory - try multiple locations
static std::string findResourcePath() {
    // 1. Try compile-time DATADIR (for system installations)
    std::string systemPath = std::string(INFINITE_TUX_DATADIR) + "resources/";
    if (checkResourcePath(systemPath)) {
        DEBUG_PRINT("Found resources at: %s", systemPath.c_str());
        return systemPath;
    }
    
    // 2. Try current directory (for development/portable builds)
    if (checkResourcePath("resources/")) {
        DEBUG_PRINT("Found resources at: resources/");
        return "resources/";
    }
    
    // 3. Try relative to executable (common on some systems)
    if (checkResourcePath("./resources/")) {
        DEBUG_PRINT("Found resources at: ./resources/");
        return "./resources/";
    }
    
    // 4. Fallback - return default and let it fail with a clear error
    std::cerr << "ERROR: Could not find resources directory!" << std::endl;
    std::cerr << "Tried:" << std::endl;
    if (strlen(INFINITE_TUX_DATADIR) > 0) {
        std::cerr << "  - " << systemPath << std::endl;
    }
    std::cerr << "  - resources/" << std::endl;
    return "resources/";
}

Game::Game() {}

Game::~Game() {
    cleanup();
}

bool Game::init(bool useDefaultBindings) {
    DEBUG_PRINT("init() starting...");
    
    // Initialize SDL
    DEBUG_PRINT("Initializing SDL...");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return false;
    }
    DEBUG_PRINT("SDL initialized OK");
    
    // Initialize SDL_image
    DEBUG_PRINT("Initializing SDL_image...");
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cerr << "SDL_image initialization failed: " << IMG_GetError() << std::endl;
        return false;
    }
    DEBUG_PRINT("SDL_image initialized OK");
    
    // Initialize SDL_mixer
    DEBUG_PRINT("Initializing SDL_mixer...");
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "SDL_mixer initialization failed: " << Mix_GetError() << std::endl;
        // Continue without sound
    }
    Mix_AllocateChannels(32);
    DEBUG_PRINT("SDL_mixer initialized OK");
    
    // Initialize input configuration
    DEBUG_PRINT("Initializing InputConfig...");
    if (!INPUTCFG.init(useDefaultBindings)) {
        std::cerr << "Warning: Failed to initialize input configuration" << std::endl;
        // Continue anyway - will use defaults
    }
    DEBUG_PRINT("InputConfig initialized OK");
    
    // Determine initial fullscreen state
    // --default flag forces windowed mode, otherwise use saved config
    fullscreenMode = useDefaultBindings ? false : INPUTCFG.isFullscreen();
    
    // Set video driver hint for Wine compatibility
    // Wine may have issues with certain SDL2 video backends
    DEBUG_PRINT("Setting video driver hints for compatibility...");
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
    // Disable batching which can cause issues in some environments
    SDL_SetHint(SDL_HINT_RENDER_BATCHING, "0");
    
    // Create window (start windowed, then switch to fullscreen if needed)
    DEBUG_PRINT("Creating window...");
    Uint32 windowFlags = SDL_WINDOW_SHOWN;
    if (fullscreenMode) {
        windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
    
    window = SDL_CreateWindow(
        "Infinite Tux",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH * WINDOW_SCALE,
        SCREEN_HEIGHT * WINDOW_SCALE,
        windowFlags
    );
    
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        return false;
    }
    DEBUG_PRINT("Window created OK");
    
    // Set texture filtering to nearest for sharp pixelated look (default)
    // "0" = nearest (pixelated), "1" = linear (smooth), "2" = anisotropic
    // Press F10 to cycle through options
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    
    // Create renderer
    DEBUG_PRINT("Creating renderer...");
    
    // Force software rendering for Wine compatibility
    // Wine's OpenGL/Direct3D emulation can cause stack smashing in SDL2
    DEBUG_PRINT("Setting render driver hint to software...");
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
    
    DEBUG_PRINT("Calling SDL_CreateRenderer with SOFTWARE flag...");
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    
    DEBUG_PRINT("SDL_CreateRenderer returned");
    
    if (!renderer) {
        DEBUG_PRINT("Software renderer failed: %s", SDL_GetError());
        DEBUG_PRINT("Trying default renderer...");
        SDL_SetHint(SDL_HINT_RENDER_DRIVER, "");
        renderer = SDL_CreateRenderer(window, -1, 0);
    }
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        return false;
    }
    DEBUG_PRINT("Renderer created OK");
    
    // Set up viewport to maintain aspect ratio
    DEBUG_PRINT("Updating viewport...");
    updateViewport();
    
    // Create screen texture for rendering
    DEBUG_PRINT("Creating screen texture...");
    screenTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, 
                                       SDL_TEXTUREACCESS_TARGET,
                                       SCREEN_WIDTH, SCREEN_HEIGHT);
    DEBUG_PRINT("Screen texture created OK");
    
    // Load resources
    DEBUG_PRINT("Loading Art resources...");
    std::string resourcePath = findResourcePath();
    if (!Art::init(renderer, resourcePath)) {
        std::cerr << "Failed to load resources!" << std::endl;
        return false;
    }
    DEBUG_PRINT("Art resources loaded OK");
    
    // Load saved volume settings from config
    DEBUG_PRINT("Initializing volume from config...");
    Art::initVolumeFromConfig();
    
    // Load tile behaviors (with user override support)
    DEBUG_PRINT("Loading tile behaviors...");
    if (!Level::loadBehaviors(Art::resolveResource("tiles.dat"))) {
        std::cerr << "Failed to load tile behaviors!" << std::endl;
        return false;
    }
    DEBUG_PRINT("Tile behaviors loaded OK");
    
    // Create map scene
    Random random;
    mapScene = new MapScene(this, random.nextLong());
    
    adjustFPS();
    
    return true;
}

void Game::run() {
    running = true;
    
    // Start with title screen (direct call is safe here - no scene exists yet)
    doSceneChange(PendingScene::TITLE);
    
    Uint32 lastTime = SDL_GetTicks();
    
    while (running) {
        Uint32 currentTime = SDL_GetTicks();
        
        handleEvents();
        updateGameInput();
        
        if (scene) {
            scene->tick();
        }
        
        // Process any pending scene change AFTER tick completes
        // This prevents use-after-free when a scene triggers its own deletion
        processPendingSceneChange();
        
        // Update input state for next frame (MUST be called after all input checks)
        INPUTCFG.updatePreviousState();
        
        // Render
        SDL_SetRenderTarget(renderer, screenTexture);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        
        if (scene) {
            scene->render(renderer, 0);
        }
        
        SDL_SetRenderTarget(renderer, nullptr);
        SDL_RenderCopy(renderer, screenTexture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
        
        // Frame timing
        if (delay > 0) {
            Uint32 frameTime = SDL_GetTicks() - currentTime;
            if (frameTime < (Uint32)delay) {
                SDL_Delay(delay - frameTime);
            }
        }
    }
    
    Art::stopMusic();
}

void Game::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // Pass events to input system
        INPUTCFG.processEvent(event);
        
        switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_KEYDOWN:
                // Handle special keys that aren't configurable
                switch (event.key.keysym.sym) {
                    case SDLK_F1:
                        useScale2x = !useScale2x;
                        break;
                    case SDLK_F5:
                        Art::adjustSfxVolume(-16);   // Decrease SFX volume
                        break;
                    case SDLK_F6:
                        Art::adjustSfxVolume(16);    // Increase SFX volume
                        break;
                    case SDLK_F7:
                        Art::adjustMusicVolume(-16);  // Decrease volume
                        break;
                    case SDLK_F8:
                        Art::adjustMusicVolume(16);   // Increase volume
                        break;
                    case SDLK_F9:
                        Art::cycleMidiSynth();
                        break;
                    case SDLK_F10:
                        cycleScaleQuality();
                        break;
                    case SDLK_F11:
                        toggleFullscreen();
                        break;
                    case SDLK_ESCAPE:
                        running = false;
                        break;
                    // Test mode keys
                    case SDLK_BACKQUOTE:
                        if (g_testMode) {
                            g_testInvincible = !g_testInvincible;
                            DEBUG_PRINT("Test invincibility %s", g_testInvincible ? "ON" : "OFF");
                        }
                        break;
                    case SDLK_i:
                        if (g_testMode && scene) scene->handleTestKey('i');
                        break;
                    case SDLK_o:
                        if (g_testMode && scene) scene->handleTestKey('o');
                        break;
                    case SDLK_p:
                        if (g_testMode && scene) scene->handleTestKey('p');
                        break;
                    case SDLK_0: case SDLK_1: case SDLK_2: case SDLK_3: case SDLK_4:
                    case SDLK_5: case SDLK_6: case SDLK_7: case SDLK_8: case SDLK_9:
                        if (g_testMode && scene) {
                            scene->handleTestKey('0' + (event.key.keysym.sym - SDLK_0));
                        }
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
}

void Game::updateGameInput() {
    if (!scene) return;
    
    // Update scene's key state from input configuration
    scene->toggleKey(Mario::KEY_LEFT, INPUTCFG.isActionPressed(GameAction::MOVE_LEFT));
    scene->toggleKey(Mario::KEY_RIGHT, INPUTCFG.isActionPressed(GameAction::MOVE_RIGHT));
    scene->toggleKey(Mario::KEY_UP, INPUTCFG.isActionPressed(GameAction::MOVE_UP));
    scene->toggleKey(Mario::KEY_DOWN, INPUTCFG.isActionPressed(GameAction::MOVE_DOWN));
    scene->toggleKey(Mario::KEY_JUMP, INPUTCFG.isActionPressed(GameAction::JUMP));
    scene->toggleKey(Mario::KEY_SPEED, INPUTCFG.isActionPressed(GameAction::FIRE));
    
    // Handle pause (only on just pressed, not held)
    if (INPUTCFG.isActionJustPressed(GameAction::PAUSE)) {
        scene->handlePauseKey();
    }
}

void Game::startLevel(long seed, int difficulty, int type) {
    pendingScene = PendingScene::LEVEL;
    pendingLevelSeed = seed;
    pendingLevelDifficulty = difficulty;
    pendingLevelType = type;
}

void Game::levelFailed() {
    // Queue return to map scene - will be processed after LevelScene::tick() completes
    pendingScene = PendingScene::LEVEL_FAILED;
}

void Game::levelWon() {
    // Queue return to map scene with level won - will be processed after LevelScene::tick() completes
    pendingScene = PendingScene::LEVEL_WON;
}

void Game::win() {
    pendingScene = PendingScene::WIN;
}

void Game::lose() {
    pendingScene = PendingScene::LOSE;
}

void Game::toTitle() {
    pendingScene = PendingScene::TITLE;
}

void Game::toOptions() {
    pendingScene = PendingScene::OPTIONS;
}

void Game::startGame() {
    pendingScene = PendingScene::START_GAME;
}

void Game::processPendingSceneChange() {
    if (pendingScene == PendingScene::NONE) {
        return;
    }
    
    PendingScene sceneToChange = pendingScene;
    pendingScene = PendingScene::NONE;  // Clear before processing
    
    doSceneChange(sceneToChange);
}

void Game::doSceneChange(PendingScene sceneType) {
    switch (sceneType) {
        case PendingScene::TITLE:
            DEBUG_PRINT("Changing to Title scene");
            Mario::resetStatic();
            if (scene && scene != mapScene) {
                delete scene;
            }
            scene = new TitleScene(this);
            scene->init();
            break;
            
        case PendingScene::OPTIONS:
            DEBUG_PRINT("Changing to Options scene");
            if (scene && scene != mapScene) {
                delete scene;
            }
            scene = new OptionsScene(this);
            scene->init();
            break;
            
        case PendingScene::START_GAME:
            DEBUG_PRINT("Starting game");
            if (scene && scene != mapScene) {
                delete scene;
            }
            scene = mapScene;
            mapScene->startMusic();
            mapScene->init();
            break;
            
        case PendingScene::WIN:
            DEBUG_PRINT("Changing to Win scene");
            if (scene && scene != mapScene) {
                delete scene;
            }
            scene = new WinScene(this);
            scene->init();
            break;
            
        case PendingScene::LOSE:
            DEBUG_PRINT("Changing to Lose scene");
            if (scene && scene != mapScene) {
                delete scene;
            }
            scene = new LoseScene(this);
            scene->init();
            break;
            
        case PendingScene::LEVEL:
            DEBUG_PRINT("Starting level");
            if (scene && scene != mapScene) {
                delete scene;
            }
            scene = new LevelScene(this, pendingLevelSeed, pendingLevelDifficulty, pendingLevelType);
            scene->init();
            break;
            
        case PendingScene::LEVEL_FAILED:
            DEBUG_PRINT("Level failed - returning to map");
            if (scene && scene != mapScene) {
                delete scene;
            }
            scene = mapScene;
            mapScene->startMusic();
            Mario::lives--;
            if (Mario::lives == 0) {
                // Queue lose scene for next frame
                pendingScene = PendingScene::LOSE;
            }
            break;
            
        case PendingScene::LEVEL_WON:
            DEBUG_PRINT("Level won - returning to map");
            if (scene && scene != mapScene) {
                delete scene;
            }
            scene = mapScene;
            mapScene->startMusic();
            mapScene->levelWon();
            break;
            
        case PendingScene::NONE:
            break;
    }
}

void Game::adjustFPS() {
    int fps = TICKS_PER_SECOND;
    delay = (fps > 0) ? ((fps >= 100) ? 0 : (1000 / fps)) : 100;
}

void Game::setFullscreen(bool fullscreen) {
    if (fullscreenMode == fullscreen) return;
    
    fullscreenMode = fullscreen;
    
    if (fullscreen) {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    } else {
        SDL_SetWindowFullscreen(window, 0);
        // Reset window size when going back to windowed
        SDL_SetWindowSize(window, SCREEN_WIDTH * WINDOW_SCALE, SCREEN_HEIGHT * WINDOW_SCALE);
        SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    }
    
    // Update viewport to maintain aspect ratio
    updateViewport();
    
    // Save setting
    INPUTCFG.setFullscreen(fullscreen);
    INPUTCFG.saveConfig();
}

void Game::toggleFullscreen() {
    setFullscreen(!fullscreenMode);
}

void Game::cycleScaleQuality() {
    scaleQuality = (scaleQuality + 1) % 3;
    
    const char* qualityStr;
    const char* qualityName;
    switch (scaleQuality) {
        case 0:
            qualityStr = "0";
            qualityName = "Nearest (Pixelated)";
            break;
        case 1:
            qualityStr = "1";
            qualityName = "Linear (Smooth)";
            break;
        case 2:
            qualityStr = "2";
            qualityName = "Best (Anisotropic)";
            break;
        default:
            qualityStr = "1";
            qualityName = "Linear (Smooth)";
            break;
    }
    
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, qualityStr);
    
    // Recreate screen texture with new quality setting
    if (screenTexture) {
        SDL_DestroyTexture(screenTexture);
    }
    screenTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                      SDL_TEXTUREACCESS_TARGET,
                                      SCREEN_WIDTH, SCREEN_HEIGHT);
    
    DEBUG_PRINT("Scale quality: %s", qualityName);
}

void Game::updateViewport() {
    int windowW, windowH;
    SDL_GetWindowSize(window, &windowW, &windowH);
    
    // Calculate the largest viewport that maintains aspect ratio
    float targetAspect = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
    float windowAspect = (float)windowW / (float)windowH;
    
    int viewportW, viewportH;
    int viewportX, viewportY;
    
    if (windowAspect > targetAspect) {
        // Window is wider than target - pillarbox (black bars on sides)
        viewportH = windowH;
        viewportW = (int)(windowH * targetAspect);
        viewportX = (windowW - viewportW) / 2;
        viewportY = 0;
    } else {
        // Window is taller than target - letterbox (black bars on top/bottom)
        viewportW = windowW;
        viewportH = (int)(windowW / targetAspect);
        viewportX = 0;
        viewportY = (windowH - viewportH) / 2;
    }
    
    // Set the viewport
    SDL_Rect viewport = {viewportX, viewportY, viewportW, viewportH};
    SDL_RenderSetViewport(renderer, &viewport);
    
    // Set logical size to maintain pixel-perfect scaling within viewport
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
}

void Game::cleanup() {
    if (scene && scene != mapScene) {
        delete scene;
        scene = nullptr;
    }
    if (mapScene) {
        delete mapScene;
        mapScene = nullptr;
    }
    
    INPUTCFG.cleanup();
    Art::cleanup();
    
    if (screenTexture) {
        SDL_DestroyTexture(screenTexture);
        screenTexture = nullptr;
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    
    Mix_CloseAudio();
    IMG_Quit();
    SDL_Quit();
}

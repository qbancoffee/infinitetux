/**
 * @file TitleScene.h
 * @brief Title screen / main menu.
 * @ingroup scenes
 * 
 * TitleScene displays the game logo and menu options.
 */
#pragma once
#include "Scene.h"
#include <string>

class BgRenderer;

class TitleScene : public Scene {
public:
    TitleScene(Game* game);
    ~TitleScene();
    
    void init() override;
    void tick() override;
    void render(SDL_Renderer* renderer, float alpha) override;

private:
    int tickCount = 0;
    bool wasDown = true;
    BgRenderer* bgLayer0 = nullptr;
    BgRenderer* bgLayer1 = nullptr;
    
    // Menu
    int selectedOption = 0;
    static constexpr int NUM_OPTIONS = 3;  // Start Game, Fullscreen, Options
    int inputDelay = 0;
    static constexpr int INPUT_DELAY_FRAMES = 6;
    
    void handleMenuInput();
    void selectOption();
    void drawText(SDL_Renderer* renderer, const std::string& text, int x, int y, bool selected = false, int scale = 1);
    void drawCenteredText(SDL_Renderer* renderer, const std::string& text, int y, bool selected = false, int scale = 1);
    std::string getFullscreenLabel() const;
};

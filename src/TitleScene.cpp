/**
 * @file TitleScene.cpp
 * @brief Title screen implementation.
 */
#include "TitleScene.h"
#include "Game.h"
#include "Art.h"
#include "Mario.h"
#include "BgRenderer.h"
#include "InputConfig.h"
#include "OptionsScene.h"
#include <cmath>
#include <cstring>

TitleScene::TitleScene(Game* game) {
    this->game = game;
    
    // Create background renderers (they generate their own levels internally)
    // distance=1 for near layer, distance=2 for far layer
    // For title screen: both are overground type
    bgLayer0 = new BgRenderer(320, 240, 0, 1, false);  // Near hills (not distant)
    bgLayer1 = new BgRenderer(320, 240, 0, 2, true);   // Far hills (distant)
}

TitleScene::~TitleScene() {
    delete bgLayer0;
    delete bgLayer1;
}

void TitleScene::init() {
    Art::startMusic(MUSIC_TITLE);
    tickCount = 0;
    wasDown = true;  // Prevent immediate trigger if key held
    selectedOption = 0;
    inputDelay = INPUT_DELAY_FRAMES;
}

void TitleScene::tick() {
    tickCount++;
    
    if (inputDelay > 0) {
        inputDelay--;
    }
    
    handleMenuInput();
    
    // Note: INPUTCFG.updatePreviousState() is called in Game::run() after all scene ticks
}

void TitleScene::handleMenuInput() {
    if (inputDelay > 0) return;
    
    // Check for up/down navigation
    bool up = INPUTCFG.isActionJustPressed(GameAction::MOVE_UP) ||
              INPUTCFG.isKeyJustPressed(SDLK_w);
    bool down = INPUTCFG.isActionJustPressed(GameAction::MOVE_DOWN) ||
                INPUTCFG.isKeyJustPressed(SDLK_s);
    bool select = INPUTCFG.isActionJustPressed(GameAction::MENU_SELECT) ||
                  INPUTCFG.isActionJustPressed(GameAction::JUMP) ||
                  INPUTCFG.isKeyJustPressed(SDLK_SPACE);
    
    if (up) {
        selectedOption--;
        if (selectedOption < 0) selectedOption = NUM_OPTIONS - 1;
        inputDelay = INPUT_DELAY_FRAMES;
    } else if (down) {
        selectedOption++;
        if (selectedOption >= NUM_OPTIONS) selectedOption = 0;
        inputDelay = INPUT_DELAY_FRAMES;
    } else if (select) {
        selectOption();
        inputDelay = INPUT_DELAY_FRAMES;
    }
}

void TitleScene::selectOption() {
    switch (selectedOption) {
        case 0:  // Start Game
            game->startGame();
            break;
        case 1:  // Fullscreen toggle
            game->toggleFullscreen();
            break;
        case 2:  // Options
            game->toOptions();
            break;
    }
}

std::string TitleScene::getFullscreenLabel() const {
    if (game->isFullscreen()) {
        return "Fullscreen: ON";
    } else {
        return "Fullscreen: OFF";
    }
}

void TitleScene::render(SDL_Renderer* renderer, float alpha) {
    // Draw sky blue background
    SDL_SetRenderDrawColor(renderer, 92, 148, 252, 255);
    SDL_RenderClear(renderer);
    
    // Render background layers (scrolling)
    if (bgLayer0 && bgLayer1) {
        bgLayer0->setCam(tickCount + 160, 0);
        bgLayer1->setCam(tickCount + 160, 0);
        bgLayer1->render(renderer, tickCount);
        bgLayer0->render(renderer, tickCount);
    }
    
    // Calculate bounce offset - sine wave oscillation (like original Java)
    float tick = tickCount + alpha;
    int yo = 16 - (int)(std::abs(std::sin(tick / 6.0) * 8));
    
    // Draw logo with bounce at position (0, yo)
    if (Art::logo) {
        int logoW, logoH;
        SDL_QueryTexture(Art::logo, nullptr, nullptr, &logoW, &logoH);
        SDL_Rect dst = {0, yo, logoW, logoH};
        SDL_RenderCopy(renderer, Art::logo, nullptr, &dst);
    }
    
    // Draw title screen image at position (0, 120)
    if (Art::titleScreen) {
        int titleW, titleH;
        SDL_QueryTexture(Art::titleScreen, nullptr, nullptr, &titleW, &titleH);
        SDL_Rect dst = {0, 120, titleW, titleH};
        SDL_RenderCopy(renderer, Art::titleScreen, nullptr, &dst);
    }
    
    // Draw menu options with normal font (scale 1)
    int fontScale = 1;
    int lineHeight = 12;  // Spacing between menu items
    
    // Position menu so that the LAST option is centered on screen
    int menuY = SCREEN_HEIGHT / 2 - (NUM_OPTIONS - 1) * lineHeight;
    
    // Menu items (dynamic label for fullscreen)
    std::string fullscreenLabel = getFullscreenLabel();
    const char* options[NUM_OPTIONS];
    options[0] = "Start Game";
    options[1] = fullscreenLabel.c_str();
    options[2] = "Options";
    
    for (int i = 0; i < NUM_OPTIONS; i++) {
        bool selected = (i == selectedOption);
        int optionLen = strlen(options[i]);
        
        // Draw selector arrow
        if (selected) {
            int arrowX = (SCREEN_WIDTH - optionLen * 8 * fontScale) / 2 - 12;
            drawText(renderer, ">", arrowX, menuY + i * lineHeight, true, fontScale);
        }
        
        drawCenteredText(renderer, options[i], menuY + i * lineHeight, selected, fontScale);
    }
    
    // Draw navigation hint at bottom (normal size)
    std::string hint = "UP/DOWN: Select  ";
    if (INPUTCFG.hasGamepad()) {
        hint += "A: Confirm";
    } else {
        hint += "ENTER: Confirm";
    }
    drawCenteredText(renderer, hint, SCREEN_HEIGHT - 20, false, 1);
}

void TitleScene::drawText(SDL_Renderer* renderer, const std::string& text, int x, int y, bool selected, int scale) {
    if (Art::font.empty()) return;
    
    int charWidth = 8 * scale;
    int charHeight = 8 * scale;
    
    // Font has 96 columns (characters) and 8 rows (colors)
    // Row 0 = black, 1 = red, 2 = green, 3 = cyan
    // Row 4 = yellow, 5 = magenta/purple, 6 = ?, 7 = white
    int colorRow = selected ? 5 : 1;  // Purple for selected, red for normal
    
    for (size_t i = 0; i < text.length(); i++) {
        char c = text[i];
        if (c < 32 || c > 127) c = '?';
        
        int charIndex = c - 32;  // Character column (0-95)
        
        if (charIndex >= 0 && charIndex < (int)Art::font.size()) {
            // font[charIndex][colorRow] gives us the character in that color
            if (colorRow < (int)Art::font[charIndex].size()) {
                SDL_Texture* tex = Art::font[charIndex][colorRow];
                if (tex) {
                    SDL_Rect dst = {x + (int)i * charWidth, y, charWidth, charHeight};
                    SDL_RenderCopy(renderer, tex, nullptr, &dst);
                }
            }
        }
    }
}

void TitleScene::drawCenteredText(SDL_Renderer* renderer, const std::string& text, int y, bool selected, int scale) {
    int charWidth = 8 * scale;
    int x = (SCREEN_WIDTH - (int)text.length() * charWidth) / 2;
    drawText(renderer, text, x, y, selected, scale);
}

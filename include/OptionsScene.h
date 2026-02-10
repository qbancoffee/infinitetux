/**
 * @file OptionsScene.h
 * @brief Options/controls configuration.
 * @ingroup scenes
 * 
 * OptionsScene allows rebinding controls and
 * configuring display settings.
 */
#pragma once
#include "Scene.h"
#include "InputConfig.h"
#include <vector>
#include <string>
#include <functional>

class Game;
class BgRenderer;

/**
 * OptionsScene - Input configuration menu
 * 
 * Menu structure:
 * - Configure Gamepad
 *   - Manual Mapping
 *   - Autoconfig
 *   - Back
 * - Configure Keyboard
 *   - (List of actions to remap)
 *   - Back
 * - Reset All to Default
 * - Back
 */

// Menu item structure
struct MenuItem {
    std::string label;
    std::function<void()> action;
    bool enabled = true;
    std::string displayValue;  // For showing current binding
};

// Menu state enum
enum class MenuState {
    MAIN_OPTIONS,
    GAMEPAD_MENU,
    GAMEPAD_MANUAL,
    GAMEPAD_AUTOCONFIG,
    KEYBOARD_MENU,
    KEYBOARD_MANUAL,
    AUDIO_MENU,
    SOUNDFONT_SELECT,
    WAITING_FOR_INPUT
};

class OptionsScene : public Scene {
public:
    OptionsScene(Game* game);
    ~OptionsScene();
    
    void init() override;
    void tick() override;
    void render(SDL_Renderer* renderer, float alpha) override;
    
private:
    Game* game;
    BgRenderer* bgLayer0 = nullptr;
    BgRenderer* bgLayer1 = nullptr;
    
    // Menu state
    MenuState menuState = MenuState::MAIN_OPTIONS;
    std::vector<MenuItem> currentMenu;
    int selectedIndex = 0;
    int tickCount = 0;
    
    // Input delay for menu navigation
    int inputDelay = 0;
    static constexpr int INPUT_DELAY_FRAMES = 6;
    
    // Waiting for input state
    GameAction waitingForAction = GameAction::COUNT;
    bool waitingForGamepad = false;
    int waitingTimeout = 0;
    int waitingInitialDelay = 0;  // Brief delay before accepting input (to let player release button)
    std::string waitingPrompt;
    
    // Autoconfig state
    int autoconfigStep = 0;
    bool inAutoconfig = false;  // Track if we're in autoconfig sequence
    static const GameAction autoconfigActions[];
    static const int AUTOCONFIG_ACTION_COUNT;
    
    // Menu building
    void buildMainMenu();
    void buildGamepadMenu();
    void buildGamepadManualMenu();
    void buildKeyboardMenu();
    void buildKeyboardManualMenu();
    void buildAudioMenu();
    void buildSoundfontSelectMenu(int trackIndex);
    
    // Soundfont selection state
    int soundfontTrackIndex = -1;  // Track being configured (-1 = default)
    std::vector<std::string> availableSoundfonts;
    int lastAudioMenuSelection = -1;  // Track selection changes for audio preview
    
    // Helper to get music index from audio menu selection
    int getMusicIndexFromAudioMenuSelection(int selection) const;
    void updateAudioPreview();  // Play music when selection changes in audio menu
    
    // Menu navigation
    void navigateUp();
    void navigateDown();
    void selectItem();
    void goBack();
    
    // Input handling
    void handleMenuInput();
    void startWaitingForInput(GameAction action, bool forGamepad);
    void checkWaitingInput();
    
    // Autoconfig
    void startAutoconfig();
    void autoconfigNextStep();
    
    // Drawing helpers
    void drawText(SDL_Renderer* renderer, const std::string& text, int x, int y, bool selected = false, int scale = 1);
    void drawCenteredText(SDL_Renderer* renderer, const std::string& text, int y, bool selected = false, int scale = 1);
    void drawMenu(SDL_Renderer* renderer);
    void drawWaitingPrompt(SDL_Renderer* renderer);
};

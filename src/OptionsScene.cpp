/**
 * @file OptionsScene.cpp
 * @brief Options menu implementation.
 */
#include "OptionsScene.h"
#include "Game.h"
#include "Art.h"
#include "BgRenderer.h"
#include "Mario.h"

// Actions to configure in autoconfig mode
const GameAction OptionsScene::autoconfigActions[] = {
    GameAction::MOVE_LEFT,
    GameAction::MOVE_RIGHT,
    GameAction::MOVE_UP,
    GameAction::MOVE_DOWN,
    GameAction::JUMP,
    GameAction::FIRE,
    GameAction::PAUSE,
    GameAction::MENU_SELECT,
    GameAction::MENU_BACK
};
const int OptionsScene::AUTOCONFIG_ACTION_COUNT = sizeof(autoconfigActions) / sizeof(autoconfigActions[0]);

OptionsScene::OptionsScene(Game* game) : game(game) {
    bgLayer0 = new BgRenderer(320, 240, 0, 1, false);
    bgLayer1 = new BgRenderer(320, 240, 0, 2, true);
}

OptionsScene::~OptionsScene() {
    delete bgLayer0;
    delete bgLayer1;
}

void OptionsScene::init() {
    Art::stopMusic();  // Stop music - audio menu will control playback
    tickCount = 0;
    selectedIndex = 0;
    inputDelay = INPUT_DELAY_FRAMES;  // Prevent immediate input
    inAutoconfig = false;
    waitingInitialDelay = 0;
    buildMainMenu();
}

void OptionsScene::tick() {
    tickCount++;
    
    if (inputDelay > 0) {
        inputDelay--;
    }
    
    if (menuState == MenuState::WAITING_FOR_INPUT) {
        checkWaitingInput();
    } else {
        handleMenuInput();
    }
    
    // Update audio preview when in audio menu
    if (menuState == MenuState::AUDIO_MENU) {
        updateAudioPreview();
    }
    
    // Note: INPUTCFG.updatePreviousState() is called in Game::run() after all scene ticks
}

void OptionsScene::render(SDL_Renderer* renderer, float alpha) {
    // Draw sky background
    SDL_SetRenderDrawColor(renderer, 92, 148, 252, 255);
    SDL_RenderClear(renderer);
    
    // Render scrolling background
    if (bgLayer0 && bgLayer1) {
        bgLayer0->setCam(tickCount + 160, 0);
        bgLayer1->setCam(tickCount + 160, 0);
        bgLayer1->render(renderer, tickCount);
        bgLayer0->render(renderer, tickCount);
    }
    
    // Draw semi-transparent overlay for readability
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
    SDL_Rect overlay = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderFillRect(renderer, &overlay);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    
    // Draw title
    std::string title;
    switch (menuState) {
        case MenuState::MAIN_OPTIONS:
            title = "OPTIONS";
            break;
        case MenuState::GAMEPAD_MENU:
            title = "GAMEPAD OPTIONS";
            break;
        case MenuState::GAMEPAD_MANUAL:
            title = "GAMEPAD MAPPING";
            break;
        case MenuState::GAMEPAD_AUTOCONFIG:
            title = "GAMEPAD AUTOCONFIG";
            break;
        case MenuState::KEYBOARD_MENU:
        case MenuState::KEYBOARD_MANUAL:
            title = "KEYBOARD MAPPING";
            break;
        case MenuState::WAITING_FOR_INPUT:
            title = waitingForGamepad ? "GAMEPAD CONFIG" : "KEYBOARD CONFIG";
            break;
    }
    drawCenteredText(renderer, title, 20, false);
    
    // Draw content
    if (menuState == MenuState::WAITING_FOR_INPUT) {
        drawWaitingPrompt(renderer);
    } else {
        drawMenu(renderer);
    }
    
    // Draw controller status
    std::string controllerStatus = INPUTCFG.hasGamepad() ? 
        ("Controller: " + INPUTCFG.getGamepadName()) : "No controller connected";
    drawText(renderer, controllerStatus, 10, SCREEN_HEIGHT - 20, false);
}

void OptionsScene::buildMainMenu() {
    menuState = MenuState::MAIN_OPTIONS;
    currentMenu.clear();
    selectedIndex = 0;
    
    currentMenu.push_back({"Configure Gamepad", [this]() {
        buildGamepadMenu();
    }, true, ""});
    
    currentMenu.push_back({"Configure Keyboard", [this]() {
        buildKeyboardMenu();
    }, true, ""});
    
    currentMenu.push_back({"Audio Settings", [this]() {
        buildAudioMenu();
    }, true, ""});
    
    currentMenu.push_back({"Reset All to Default", [this]() {
        INPUTCFG.resetToDefaults();
        // Rebuild current menu to show updated values
        buildMainMenu();
    }, true, ""});
    
    currentMenu.push_back({"Back", [this]() {
        INPUTCFG.saveConfig();
        game->toTitle();
    }, true, ""});
}

void OptionsScene::buildGamepadMenu() {
    menuState = MenuState::GAMEPAD_MENU;
    currentMenu.clear();
    selectedIndex = 0;
    
    currentMenu.push_back({"Manual Mapping", [this]() {
        buildGamepadManualMenu();
    }, INPUTCFG.hasGamepad(), ""});
    
    currentMenu.push_back({"Autoconfig", [this]() {
        startAutoconfig();
    }, INPUTCFG.hasGamepad(), ""});
    
    currentMenu.push_back({"Back", [this]() {
        buildMainMenu();
    }, true, ""});
    
    // Update enabled state based on controller connection
    for (auto& item : currentMenu) {
        if (item.label == "Manual Mapping" || item.label == "Autoconfig") {
            item.enabled = INPUTCFG.hasGamepad();
        }
    }
}

void OptionsScene::buildGamepadManualMenu() {
    menuState = MenuState::GAMEPAD_MANUAL;
    currentMenu.clear();
    selectedIndex = 0;
    
    // Add each action that can be bound
    for (int i = 0; i < static_cast<int>(GameAction::COUNT); i++) {
        GameAction action = static_cast<GameAction>(i);
        std::string displayName = InputConfig::getActionDisplayName(action);
        InputBinding binding = INPUTCFG.getGamepadBinding(action);
        std::string bindingStr = binding.isValid() ? InputConfig::bindingToString(binding) : "Not Set";
        
        currentMenu.push_back({displayName, [this, action]() {
            startWaitingForInput(action, true);
        }, true, bindingStr});
    }
    
    currentMenu.push_back({"Back", [this]() {
        INPUTCFG.saveConfig();
        buildGamepadMenu();
    }, true, ""});
}

void OptionsScene::buildKeyboardMenu() {
    menuState = MenuState::KEYBOARD_MENU;
    buildKeyboardManualMenu();
}

void OptionsScene::buildKeyboardManualMenu() {
    menuState = MenuState::KEYBOARD_MANUAL;
    currentMenu.clear();
    selectedIndex = 0;
    
    // Add each action that can be bound
    for (int i = 0; i < static_cast<int>(GameAction::COUNT); i++) {
        GameAction action = static_cast<GameAction>(i);
        std::string displayName = InputConfig::getActionDisplayName(action);
        InputBinding binding = INPUTCFG.getKeyboardBinding(action);
        std::string bindingStr = binding.isValid() ? InputConfig::bindingToString(binding) : "Not Set";
        
        currentMenu.push_back({displayName, [this, action]() {
            startWaitingForInput(action, false);
        }, true, bindingStr});
    }
    
    currentMenu.push_back({"Back", [this]() {
        INPUTCFG.saveConfig();
        buildMainMenu();
    }, true, ""});
}

void OptionsScene::buildAudioMenu() {
    menuState = MenuState::AUDIO_MENU;
    currentMenu.clear();
    selectedIndex = 0;
    lastAudioMenuSelection = -1;  // Reset selection tracking
    
    // Stop any playing music when entering audio menu
    Art::stopMusic();
    
    // Refresh available soundfonts list
    availableSoundfonts = INPUTCFG.getAvailableSoundfonts();
    
    // Helper to get display name for a soundfont
    auto getSfDisplay = [](const std::string& sf) -> std::string {
        if (sf.empty()) return "(System Default)";
        return sf;
    };
    
    // Default soundfont setting
    currentMenu.push_back({"Default Soundfont", [this]() {
        soundfontTrackIndex = -1;  // -1 = default
        buildSoundfontSelectMenu(-1);
    }, true, getSfDisplay(INPUTCFG.getDefaultSoundfont())});
    
    // Per-track soundfont settings
    currentMenu.push_back({"Title Music", [this]() {
        soundfontTrackIndex = MUSIC_TITLE;
        buildSoundfontSelectMenu(MUSIC_TITLE);
    }, true, getSfDisplay(INPUTCFG.getSoundfontForTrack(MUSIC_TITLE))});
    
    currentMenu.push_back({"Overworld Music", [this]() {
        soundfontTrackIndex = MUSIC_OVERWORLD;
        buildSoundfontSelectMenu(MUSIC_OVERWORLD);
    }, true, getSfDisplay(INPUTCFG.getSoundfontForTrack(MUSIC_OVERWORLD))});
    
    currentMenu.push_back({"Underground Music", [this]() {
        soundfontTrackIndex = MUSIC_UNDERGROUND;
        buildSoundfontSelectMenu(MUSIC_UNDERGROUND);
    }, true, getSfDisplay(INPUTCFG.getSoundfontForTrack(MUSIC_UNDERGROUND))});
    
    currentMenu.push_back({"Castle Music", [this]() {
        soundfontTrackIndex = MUSIC_CASTLE;
        buildSoundfontSelectMenu(MUSIC_CASTLE);
    }, true, getSfDisplay(INPUTCFG.getSoundfontForTrack(MUSIC_CASTLE))});
    
    currentMenu.push_back({"Map Music", [this]() {
        soundfontTrackIndex = MUSIC_MAP;
        buildSoundfontSelectMenu(MUSIC_MAP);
    }, true, getSfDisplay(INPUTCFG.getSoundfontForTrack(MUSIC_MAP))});
    
    currentMenu.push_back({"Back", [this]() {
        INPUTCFG.saveConfig();
        buildMainMenu();
    }, true, ""});
}

void OptionsScene::buildSoundfontSelectMenu(int trackIndex) {
    menuState = MenuState::SOUNDFONT_SELECT;
    currentMenu.clear();
    selectedIndex = 0;
    
    // Get current soundfont for this track
    std::string currentSf;
    if (trackIndex < 0) {
        currentSf = INPUTCFG.getDefaultSoundfont();
    } else {
        currentSf = INPUTCFG.getSoundfontForTrack(trackIndex);
    }
    
    // Add option for each available soundfont
    for (const auto& sf : availableSoundfonts) {
        std::string displayName = sf.empty() ? "(System Default)" : sf;
        bool isSelected = (sf == currentSf) || (sf.empty() && currentSf.empty());
        
        currentMenu.push_back({displayName, [this, trackIndex, sf]() {
            // Set the soundfont
            if (trackIndex < 0) {
                INPUTCFG.setDefaultSoundfont(sf);
            } else {
                INPUTCFG.setSoundfontForTrack(trackIndex, sf);
            }
            INPUTCFG.saveConfig();
            
            // If this track is currently playing, restart it with new soundfont
            int currentlyPlaying = Art::currentMusic;
            if (currentlyPlaying >= 0 && currentlyPlaying < MUSIC_COUNT) {
                if (currentlyPlaying == trackIndex || trackIndex < 0) {
                    Art::startMusic(currentlyPlaying);
                }
            }
            
            // Go back to audio menu
            buildAudioMenu();
        }, true, isSelected ? "<" : ""});
    }
    
    currentMenu.push_back({"Cancel", [this]() {
        buildAudioMenu();
    }, true, ""});
    
    // Start playing the track being configured
    if (trackIndex >= 0) {
        Art::startMusic(trackIndex, true);
    }
}

int OptionsScene::getMusicIndexFromAudioMenuSelection(int selection) const {
    // Audio menu order: Default, Title, Overworld, Underground, Castle, Map, Back
    switch (selection) {
        case 1: return MUSIC_TITLE;
        case 2: return MUSIC_OVERWORLD;
        case 3: return MUSIC_UNDERGROUND;
        case 4: return MUSIC_CASTLE;
        case 5: return MUSIC_MAP;
        default: return -1;  // Default soundfont or Back - no preview
    }
}

void OptionsScene::updateAudioPreview() {
    if (menuState != MenuState::AUDIO_MENU) return;
    
    // Check if selection changed
    if (selectedIndex == lastAudioMenuSelection) return;
    lastAudioMenuSelection = selectedIndex;
    
    // Get music index for current selection
    int musicIndex = getMusicIndexFromAudioMenuSelection(selectedIndex);
    
    if (musicIndex >= 0) {
        // Play the selected track
        Art::startMusic(musicIndex, true);
    } else {
        // Stop music for non-track items (Default Soundfont, Back)
        Art::stopMusic();
    }
}

void OptionsScene::navigateUp() {
    if (currentMenu.empty()) return;
    
    do {
        selectedIndex--;
        if (selectedIndex < 0) {
            selectedIndex = currentMenu.size() - 1;
        }
    } while (!currentMenu[selectedIndex].enabled && selectedIndex != 0);
}

void OptionsScene::navigateDown() {
    if (currentMenu.empty()) return;
    
    do {
        selectedIndex++;
        if (selectedIndex >= (int)currentMenu.size()) {
            selectedIndex = 0;
        }
    } while (!currentMenu[selectedIndex].enabled && selectedIndex != (int)currentMenu.size() - 1);
}

void OptionsScene::selectItem() {
    if (currentMenu.empty()) return;
    if (selectedIndex < 0 || selectedIndex >= (int)currentMenu.size()) return;
    
    MenuItem& item = currentMenu[selectedIndex];
    if (item.enabled && item.action) {
        item.action();
    }
}

void OptionsScene::goBack() {
    // Stop music preview when leaving audio-related menus
    if (menuState == MenuState::AUDIO_MENU || menuState == MenuState::SOUNDFONT_SELECT) {
        Art::stopMusic();
    }
    
    switch (menuState) {
        case MenuState::MAIN_OPTIONS:
            INPUTCFG.saveConfig();
            game->toTitle();
            break;
        case MenuState::GAMEPAD_MENU:
        case MenuState::KEYBOARD_MENU:
        case MenuState::AUDIO_MENU:
        case MenuState::KEYBOARD_MANUAL:
            buildMainMenu();
            break;
        case MenuState::SOUNDFONT_SELECT:
            buildAudioMenu();
            break;
        case MenuState::GAMEPAD_MANUAL:
            INPUTCFG.saveConfig();
            buildGamepadMenu();
            break;
        case MenuState::GAMEPAD_AUTOCONFIG:
            INPUTCFG.saveConfig();
            inAutoconfig = false;
            buildGamepadMenu();
            break;
        case MenuState::WAITING_FOR_INPUT:
            // Cancel waiting
            if (inAutoconfig) {
                INPUTCFG.saveConfig();
                inAutoconfig = false;
                buildGamepadMenu();
            } else if (waitingForGamepad) {
                buildGamepadManualMenu();
            } else {
                buildKeyboardManualMenu();
            }
            break;
    }
    inputDelay = INPUT_DELAY_FRAMES;
}

void OptionsScene::handleMenuInput() {
    if (inputDelay > 0) return;
    
    bool up = INPUTCFG.isActionJustPressed(GameAction::MOVE_UP) ||
              INPUTCFG.isKeyJustPressed(SDLK_w);
    bool down = INPUTCFG.isActionJustPressed(GameAction::MOVE_DOWN) ||
                INPUTCFG.isKeyJustPressed(SDLK_s);
    bool select = INPUTCFG.isActionJustPressed(GameAction::MENU_SELECT) ||
                  INPUTCFG.isActionJustPressed(GameAction::JUMP) ||
                  INPUTCFG.isKeyJustPressed(SDLK_SPACE);
    bool back = INPUTCFG.isActionJustPressed(GameAction::MENU_BACK) ||
                INPUTCFG.isKeyJustPressed(SDLK_ESCAPE);
    
    if (up) {
        navigateUp();
        inputDelay = INPUT_DELAY_FRAMES;
    } else if (down) {
        navigateDown();
        inputDelay = INPUT_DELAY_FRAMES;
    } else if (select) {
        selectItem();
        inputDelay = INPUT_DELAY_FRAMES;
    } else if (back) {
        goBack();
    }
}

void OptionsScene::startWaitingForInput(GameAction action, bool forGamepad) {
    menuState = MenuState::WAITING_FOR_INPUT;
    waitingForAction = action;
    waitingForGamepad = forGamepad;
    waitingTimeout = 5 * TICKS_PER_SECOND;  // 5 seconds
    waitingInitialDelay = TICKS_PER_SECOND / 2;  // 0.5 second delay before accepting input
    waitingPrompt = "Press " + std::string(forGamepad ? "button/axis" : "key") + 
                    " for: " + InputConfig::getActionDisplayName(action);
    
    // Clear any held inputs
    inputDelay = INPUT_DELAY_FRAMES;
}

void OptionsScene::checkWaitingInput() {
    // Handle initial delay (let player release any held buttons)
    if (waitingInitialDelay > 0) {
        waitingInitialDelay--;
        // Still poll events to clear them
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            // Just consume events during delay
            if (event.type == SDL_QUIT) {
                INPUTCFG.saveConfig();
                return;
            }
        }
        return;
    }
    
    waitingTimeout--;
    
    if (waitingTimeout <= 0) {
        // Timeout - return to previous menu
        if (inAutoconfig) {
            INPUTCFG.saveConfig();
            inAutoconfig = false;
            buildGamepadMenu();
        } else if (waitingForGamepad) {
            buildGamepadManualMenu();
        } else {
            buildKeyboardManualMenu();
        }
        return;
    }
    
    // Check for cancel (Escape key only - don't check gamepad B during config)
    if (INPUTCFG.isKeyJustPressed(SDLK_ESCAPE)) {
        if (inAutoconfig) {
            INPUTCFG.saveConfig();
            inAutoconfig = false;
            buildGamepadMenu();
        } else {
            goBack();
        }
        return;
    }
    
    InputBinding newBinding;
    
    if (waitingForGamepad) {
        // Poll events first to update controller state
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            INPUTCFG.processEvent(event);
            
            // Check for immediate button press from event
            if (event.type == SDL_CONTROLLERBUTTONDOWN) {
                newBinding = {InputType::GAMEPAD_BUTTON, event.cbutton.button};
            }
            // Check for axis movement from event
            else if (event.type == SDL_CONTROLLERAXISMOTION) {
                float value = event.caxis.value / 32767.0f;
                if (value > 0.5f) {
                    newBinding = {InputType::GAMEPAD_AXIS_POS, event.caxis.axis};
                } else if (value < -0.5f) {
                    newBinding = {InputType::GAMEPAD_AXIS_NEG, event.caxis.axis};
                }
            }
            // Check for hat motion
            else if (event.type == SDL_JOYHATMOTION && event.jhat.value != SDL_HAT_CENTERED) {
                newBinding = {InputType::GAMEPAD_HAT, event.jhat.value};
            }
            
            if (event.type == SDL_QUIT) {
                INPUTCFG.saveConfig();
                return;
            }
        }
        
        // If no event detected, also check current state (catches held buttons/axes)
        if (!newBinding.isValid()) {
            newBinding = INPUTCFG.waitForGamepad(0);  // Non-blocking check of current state
        }
        
        if (newBinding.isValid()) {
            INPUTCFG.setGamepadBinding(waitingForAction, newBinding);
            
            if (inAutoconfig) {
                autoconfigNextStep();
            } else {
                buildGamepadManualMenu();
            }
        }
    } else {
        // Keyboard input detection
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            INPUTCFG.processEvent(event);
            
            if (event.type == SDL_KEYDOWN) {
                SDL_Keycode key = event.key.keysym.sym;
                // Ignore modifiers and escape
                if (key != SDLK_LSHIFT && key != SDLK_RSHIFT &&
                    key != SDLK_LCTRL && key != SDLK_RCTRL &&
                    key != SDLK_LALT && key != SDLK_RALT &&
                    key != SDLK_ESCAPE) {
                    INPUTCFG.setKeyboardBinding(waitingForAction, key);
                    buildKeyboardManualMenu();
                    return;
                }
            }
            if (event.type == SDL_QUIT) {
                INPUTCFG.saveConfig();
                return;
            }
        }
    }
}

void OptionsScene::startAutoconfig() {
    if (!INPUTCFG.hasGamepad()) {
        buildGamepadMenu();
        return;
    }
    
    menuState = MenuState::GAMEPAD_AUTOCONFIG;
    autoconfigStep = 0;
    inAutoconfig = true;
    
    // Start with first action
    startWaitingForInput(autoconfigActions[0], true);
}

void OptionsScene::autoconfigNextStep() {
    autoconfigStep++;
    
    if (autoconfigStep >= AUTOCONFIG_ACTION_COUNT) {
        // Done!
        INPUTCFG.saveConfig();
        inAutoconfig = false;
        buildGamepadMenu();
    } else {
        // Next action - add small delay between steps
        startWaitingForInput(autoconfigActions[autoconfigStep], true);
    }
}

void OptionsScene::drawText(SDL_Renderer* renderer, const std::string& text, int x, int y, bool selected, int scale) {
    if (Art::font.empty()) return;
    
    int charWidth = 8 * scale;
    int charHeight = 8 * scale;
    
    // Font has 96 columns (characters) and 8 rows (colors)
    // Row 0 = black/dark, Row 7 = white/bright
    // Use row 7 for white, row 4 for yellow (selected)
    int colorRow = selected ? 4 : 7;  // Yellow for selected, white for normal
    
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

void OptionsScene::drawCenteredText(SDL_Renderer* renderer, const std::string& text, int y, bool selected, int scale) {
    int charWidth = 8 * scale;
    int x = (SCREEN_WIDTH - (int)text.length() * charWidth) / 2;
    drawText(renderer, text, x, y, selected, scale);
}

void OptionsScene::drawMenu(SDL_Renderer* renderer) {
    int startY = 50;
    int lineHeight = 16;
    
    for (size_t i = 0; i < currentMenu.size(); i++) {
        const MenuItem& item = currentMenu[i];
        bool selected = (i == (size_t)selectedIndex);
        
        // Draw selector arrow
        if (selected) {
            drawText(renderer, ">", 20, startY + i * lineHeight, true);
        }
        
        // Draw item label
        std::string displayText = item.label;
        if (!item.displayValue.empty()) {
            displayText += ": " + item.displayValue;
        }
        
        // Gray out disabled items
        if (!item.enabled) {
            // Draw in gray (we'll just not highlight it)
            drawText(renderer, displayText, 32, startY + i * lineHeight, false);
        } else {
            drawText(renderer, displayText, 32, startY + i * lineHeight, selected);
        }
    }
    
    // Draw navigation hint
    std::string hint = "UP/DOWN: Navigate  ";
    if (INPUTCFG.hasGamepad()) {
        hint += "A: Select  B: Back";
    } else {
        hint += "ENTER: Select  BACKSPACE: Back";
    }
    drawText(renderer, hint, 10, SCREEN_HEIGHT - 30, false);
}

void OptionsScene::drawWaitingPrompt(SDL_Renderer* renderer) {
    // Draw prompt in center of screen with larger font
    drawCenteredText(renderer, waitingPrompt, SCREEN_HEIGHT / 2 - 30, true, 1);
    
    // Show "waiting..." indicator during initial delay, otherwise show timeout
    if (waitingInitialDelay > 0) {
        drawCenteredText(renderer, "Get ready...", SCREEN_HEIGHT / 2, false, 1);
    } else {
        // Draw timeout countdown
        int secondsLeft = (waitingTimeout + TICKS_PER_SECOND - 1) / TICKS_PER_SECOND;
        std::string timeoutStr = "Timeout in " + std::to_string(secondsLeft) + "s";
        drawCenteredText(renderer, timeoutStr, SCREEN_HEIGHT / 2, false, 1);
    }
    
    // Draw cancel hint
    drawCenteredText(renderer, "Press ESC to cancel", SCREEN_HEIGHT / 2 + 20, false, 1);
    
    // Autoconfig progress
    if (inAutoconfig) {
        std::string progress = "Step " + std::to_string(autoconfigStep + 1) + 
                              " of " + std::to_string(AUTOCONFIG_ACTION_COUNT);
        drawCenteredText(renderer, progress, SCREEN_HEIGHT / 2 + 40, false, 1);
    }
}

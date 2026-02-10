/**
 * @file InputConfig.cpp
 * @brief Input configuration implementation.
 */
#include "InputConfig.h"
#include "Art.h"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <algorithm>
#include <cctype>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include <filesystem>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <dirent.h>
#endif

InputConfig& InputConfig::getInstance() {
    static InputConfig instance;
    return instance;
}

bool InputConfig::init(bool useDefaults) {
    DEBUG_PRINT("init() starting, useDefaults=%s", std::to_string(useDefaults).c_str());
    if (initialized) {
        DEBUG_PRINT("Already initialized, returning");
        return true;
    }
    
    useDefaultsOnly = useDefaults;
    
    // Initialize SDL game controller subsystem
    DEBUG_PRINT("Initializing SDL game controller subsystem...");
    if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK) < 0) {
        std::cerr << "Failed to initialize game controller subsystem: " << SDL_GetError() << std::endl;
        // Continue without gamepad support
    }
    DEBUG_PRINT("SDL game controller subsystem OK");
    
    // Set defaults first
    DEBUG_PRINT("Setting defaults...");
    setDefaults();
    DEBUG_PRINT("Defaults set OK");
    
    // Try to load config (unless using defaults only)
    if (!useDefaults) {
        DEBUG_PRINT("Loading config...");
        if (!loadConfig()) {
            DEBUG_PRINT("Config doesn't exist, saving defaults...");
            // Config doesn't exist, save defaults
            saveConfig();
        }
        DEBUG_PRINT("Config loaded/saved OK");
    } else {
        DEBUG_PRINT("Using default bindings (--default mode)");
    }
    
    // Try to open first available gamepad
    DEBUG_PRINT("Looking for gamepads...");
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            DEBUG_PRINT("Found gamepad at index %s", std::to_string(i).c_str());
            openGamepad(i);
            break;
        }
    }
    DEBUG_PRINT("Gamepad search complete");
    
    initialized = true;
    DEBUG_PRINT("init() complete");
    return true;
}

void InputConfig::cleanup() {
    closeGamepad();
    initialized = false;
}

std::string InputConfig::getConfigPath() const {
    DEBUG_PRINT("getConfigPath() called");
    std::string homeDir;
    
#ifdef _WIN32
    DEBUG_PRINT("Windows path detection...");
    char path[MAX_PATH];
    memset(path, 0, sizeof(path));  // Zero-initialize for safety
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, path))) {
        homeDir = path;
        DEBUG_PRINT("Got APPDATA: %s", std::to_string(homeDir).c_str());
    } else {
        homeDir = ".";
        DEBUG_PRINT("APPDATA failed, using current dir");
    }
    std::string result = homeDir + "\\infinite-tux-input.cfg";
    DEBUG_PRINT("Config path: %s", std::to_string(result).c_str());
    return result;
#else
    const char* home = getenv("HOME");
    if (!home) {
        struct passwd* pw = getpwuid(getuid());
        if (pw) {
            home = pw->pw_dir;
        }
    }
    homeDir = home ? home : ".";
    return homeDir + "/.infinite-tux-input.cfg";
#endif
}

void InputConfig::setDefaults() {
    // Clear existing bindings
    keyboardBindings.clear();
    gamepadBindings.clear();
    
    // Default keyboard bindings
    keyboardBindings[GameAction::MOVE_LEFT] = {InputType::KEYBOARD_KEY, SDLK_LEFT};
    keyboardBindings[GameAction::MOVE_RIGHT] = {InputType::KEYBOARD_KEY, SDLK_RIGHT};
    keyboardBindings[GameAction::MOVE_UP] = {InputType::KEYBOARD_KEY, SDLK_UP};
    keyboardBindings[GameAction::MOVE_DOWN] = {InputType::KEYBOARD_KEY, SDLK_DOWN};
    keyboardBindings[GameAction::JUMP] = {InputType::KEYBOARD_KEY, SDLK_x};
    keyboardBindings[GameAction::FIRE] = {InputType::KEYBOARD_KEY, SDLK_z};
    keyboardBindings[GameAction::PAUSE] = {InputType::KEYBOARD_KEY, SDLK_RETURN};
    keyboardBindings[GameAction::MENU_SELECT] = {InputType::KEYBOARD_KEY, SDLK_RETURN};
    keyboardBindings[GameAction::MENU_BACK] = {InputType::KEYBOARD_KEY, SDLK_BACKSPACE};
    
    // Default gamepad bindings (Xbox-style layout)
    gamepadBindings[GameAction::MOVE_LEFT] = {InputType::GAMEPAD_AXIS_NEG, SDL_CONTROLLER_AXIS_LEFTX};
    gamepadBindings[GameAction::MOVE_RIGHT] = {InputType::GAMEPAD_AXIS_POS, SDL_CONTROLLER_AXIS_LEFTX};
    gamepadBindings[GameAction::MOVE_UP] = {InputType::GAMEPAD_AXIS_NEG, SDL_CONTROLLER_AXIS_LEFTY};
    gamepadBindings[GameAction::MOVE_DOWN] = {InputType::GAMEPAD_AXIS_POS, SDL_CONTROLLER_AXIS_LEFTY};
    gamepadBindings[GameAction::JUMP] = {InputType::GAMEPAD_BUTTON, SDL_CONTROLLER_BUTTON_A};
    gamepadBindings[GameAction::FIRE] = {InputType::GAMEPAD_BUTTON, SDL_CONTROLLER_BUTTON_X};
    gamepadBindings[GameAction::PAUSE] = {InputType::GAMEPAD_BUTTON, SDL_CONTROLLER_BUTTON_START};
    gamepadBindings[GameAction::MENU_SELECT] = {InputType::GAMEPAD_BUTTON, SDL_CONTROLLER_BUTTON_A};
    gamepadBindings[GameAction::MENU_BACK] = {InputType::GAMEPAD_BUTTON, SDL_CONTROLLER_BUTTON_B};
}

void InputConfig::resetToDefaults() {
    setDefaults();
    if (!useDefaultsOnly) {
        saveConfig();
    }
}

bool InputConfig::loadConfig() {
    std::string path = getConfigPath();
    std::ifstream file(path);
    
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    std::string currentSection;
    
    while (std::getline(file, line)) {
        // Trim whitespace
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        size_t end = line.find_last_not_of(" \t\r\n");
        line = line.substr(start, end - start + 1);
        
        // Skip comments
        if (line.empty() || line[0] == '#' || line[0] == ';') continue;
        
        // Check for section header
        if (line[0] == '[' && line.back() == ']') {
            currentSection = line.substr(1, line.length() - 2);
            // Convert to lowercase
            std::transform(currentSection.begin(), currentSection.end(), 
                          currentSection.begin(), ::tolower);
            continue;
        }
        
        // Parse key=value
        size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) continue;
        
        std::string key = line.substr(0, eqPos);
        std::string value = line.substr(eqPos + 1);
        
        // Trim key and value
        start = key.find_first_not_of(" \t");
        end = key.find_last_not_of(" \t");
        if (start != std::string::npos) {
            key = key.substr(start, end - start + 1);
        }
        
        start = value.find_first_not_of(" \t");
        end = value.find_last_not_of(" \t");
        if (start != std::string::npos) {
            value = value.substr(start, end - start + 1);
        }
        
        // Convert key to lowercase for matching
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        
        // Handle display section
        if (currentSection == "display") {
            if (key == "fullscreen") {
                fullscreen = (value == "true" || value == "1" || value == "yes");
            }
            continue;
        }
        
        // Handle audio section
        if (currentSection == "audio") {
            if (key == "musicvolume" || key == "music_volume") {
                try {
                    musicVolume = std::stoi(value);
                    if (musicVolume < 0) musicVolume = 0;
                    if (musicVolume > 128) musicVolume = 128;
                } catch (...) {}
            } else if (key == "sfxvolume" || key == "sfx_volume") {
                try {
                    sfxVolume = std::stoi(value);
                    if (sfxVolume < 0) sfxVolume = 0;
                    if (sfxVolume > 128) sfxVolume = 128;
                } catch (...) {}
            }
            continue;
        }
        
        // Handle soundfonts section
        if (currentSection == "soundfonts") {
            if (key == "default") {
                defaultSoundfont = value;
            } else if (key == "title") {
                trackSoundfonts[MUSIC_TITLE] = value;
            } else if (key == "overworld") {
                trackSoundfonts[MUSIC_OVERWORLD] = value;
            } else if (key == "underground") {
                trackSoundfonts[MUSIC_UNDERGROUND] = value;
            } else if (key == "castle") {
                trackSoundfonts[MUSIC_CASTLE] = value;
            } else if (key == "map") {
                trackSoundfonts[MUSIC_MAP] = value;
            }
            continue;
        }
        
        GameAction action = stringToAction(key);
        if (action == GameAction::COUNT) continue;  // Unknown action
        
        InputBinding binding = stringToBinding(value);
        if (!binding.isValid()) continue;
        
        if (currentSection == "keyboard") {
            keyboardBindings[action] = binding;
        } else if (currentSection == "gamepad") {
            gamepadBindings[action] = binding;
        }
    }
    
    DEBUG_PRINT("Loaded configuration from %s", path.c_str());
    return true;
}

bool InputConfig::saveConfig() {
    if (useDefaultsOnly) return false;
    
    std::string path = getConfigPath();
    std::ofstream file(path);
    
    if (!file.is_open()) {
        std::cerr << "[INPUT] Failed to save config to " << path << std::endl;
        return false;
    }
    
    file << "# Infinite Tux Configuration\n";
    file << "# Edit this file to customize controls and display settings\n";
    file << "# Delete this file to reset to defaults\n\n";
    
    file << "[display]\n";
    file << "fullscreen = " << (fullscreen ? "true" : "false") << "\n";
    
    file << "\n[audio]\n";
    file << "music_volume = " << musicVolume << "\n";
    file << "sfx_volume = " << sfxVolume << "\n";
    
    file << "\n[soundfonts]\n";
    file << "# Soundfont files should be placed in resources/soundfonts/\n";
    file << "# Leave empty or remove line to use system default\n";
    file << "default = " << defaultSoundfont << "\n";
    
    // Helper lambda to get track soundfont or empty
    auto getTrackSf = [this](int track) -> std::string {
        auto it = trackSoundfonts.find(track);
        return (it != trackSoundfonts.end()) ? it->second : "";
    };
    
    file << "title = " << getTrackSf(MUSIC_TITLE) << "\n";
    file << "overworld = " << getTrackSf(MUSIC_OVERWORLD) << "\n";
    file << "underground = " << getTrackSf(MUSIC_UNDERGROUND) << "\n";
    file << "castle = " << getTrackSf(MUSIC_CASTLE) << "\n";
    file << "map = " << getTrackSf(MUSIC_MAP) << "\n";
    
    file << "\n[keyboard]\n";
    for (int i = 0; i < static_cast<int>(GameAction::COUNT); i++) {
        GameAction action = static_cast<GameAction>(i);
        auto it = keyboardBindings.find(action);
        if (it != keyboardBindings.end() && it->second.isValid()) {
            file << actionToString(action) << " = " << bindingToString(it->second) << "\n";
        }
    }
    
    file << "\n[gamepad]\n";
    for (int i = 0; i < static_cast<int>(GameAction::COUNT); i++) {
        GameAction action = static_cast<GameAction>(i);
        auto it = gamepadBindings.find(action);
        if (it != gamepadBindings.end() && it->second.isValid()) {
            file << actionToString(action) << " = " << bindingToString(it->second) << "\n";
        }
    }
    
    DEBUG_PRINT("Saved configuration to %s", path.c_str());
    return true;
}

void InputConfig::setKeyboardBinding(GameAction action, SDL_Keycode key) {
    keyboardBindings[action] = {InputType::KEYBOARD_KEY, key};
}

void InputConfig::setGamepadBinding(GameAction action, InputBinding binding) {
    gamepadBindings[action] = binding;
}

InputBinding InputConfig::getKeyboardBinding(GameAction action) const {
    auto it = keyboardBindings.find(action);
    if (it != keyboardBindings.end()) {
        return it->second;
    }
    return {};
}

InputBinding InputConfig::getGamepadBinding(GameAction action) const {
    auto it = gamepadBindings.find(action);
    if (it != gamepadBindings.end()) {
        return it->second;
    }
    return {};
}

void InputConfig::processEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_KEYDOWN:
            keyState[event.key.keysym.sym] = true;
            break;
        case SDL_KEYUP:
            keyState[event.key.keysym.sym] = false;
            break;
        case SDL_CONTROLLERBUTTONDOWN:
            buttonState[event.cbutton.button] = true;
            break;
        case SDL_CONTROLLERBUTTONUP:
            buttonState[event.cbutton.button] = false;
            break;
        case SDL_CONTROLLERAXISMOTION:
            axisState[event.caxis.axis] = event.caxis.value / 32767.0f;
            break;
        case SDL_CONTROLLERDEVICEADDED:
            if (!gamepad) {
                openGamepad(event.cdevice.which);
            }
            break;
        case SDL_CONTROLLERDEVICEREMOVED:
            if (gamepad && event.cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gamepad))) {
                closeGamepad();
            }
            break;
        case SDL_JOYHATMOTION:
            hatState = event.jhat.value;
            break;
    }
}

void InputConfig::updatePreviousState() {
    prevKeyState = keyState;
    prevButtonState = buttonState;
    prevHatState = hatState;
    prevActionState = actionState;
}

bool InputConfig::isActionPressed(GameAction action) const {
    bool pressed = false;
    
    // Check keyboard binding
    auto kbIt = keyboardBindings.find(action);
    if (kbIt != keyboardBindings.end() && kbIt->second.isValid()) {
        if (kbIt->second.type == InputType::KEYBOARD_KEY) {
            auto keyIt = keyState.find(kbIt->second.code);
            if (keyIt != keyState.end() && keyIt->second) {
                pressed = true;
            }
        }
    }
    
    // Check gamepad binding
    if (!pressed) {
        auto gpIt = gamepadBindings.find(action);
        if (gpIt != gamepadBindings.end() && gpIt->second.isValid() && gamepad) {
            const InputBinding& binding = gpIt->second;
            
            switch (binding.type) {
                case InputType::GAMEPAD_BUTTON: {
                    auto btnIt = buttonState.find(binding.code);
                    if (btnIt != buttonState.end() && btnIt->second) {
                        pressed = true;
                    }
                    break;
                }
                case InputType::GAMEPAD_AXIS_POS: {
                    auto axisIt = axisState.find(binding.code);
                    if (axisIt != axisState.end() && axisIt->second > AXIS_DEADZONE) {
                        pressed = true;
                    }
                    break;
                }
                case InputType::GAMEPAD_AXIS_NEG: {
                    auto axisIt = axisState.find(binding.code);
                    if (axisIt != axisState.end() && axisIt->second < -AXIS_DEADZONE) {
                        pressed = true;
                    }
                    break;
                }
                case InputType::GAMEPAD_HAT: {
                    if (hatState & binding.code) {
                        pressed = true;
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }
    
    // Also check D-pad for movement actions (always active)
    if (!pressed && gamepad) {
        switch (action) {
            case GameAction::MOVE_LEFT:
                if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_DPAD_LEFT)) {
                    pressed = true;
                }
                break;
            case GameAction::MOVE_RIGHT:
                if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_DPAD_RIGHT)) {
                    pressed = true;
                }
                break;
            case GameAction::MOVE_UP:
                if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_DPAD_UP)) {
                    pressed = true;
                }
                break;
            case GameAction::MOVE_DOWN:
                if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_DPAD_DOWN)) {
                    pressed = true;
                }
                break;
            default:
                break;
        }
    }
    
    // Update current action state for this frame
    actionState[action] = pressed;
    return pressed;
}

bool InputConfig::isActionJustPressed(GameAction action) const {
    // Get current state (this also updates actionState[action])
    bool current = isActionPressed(action);
    
    // Check against PREVIOUS frame's state (stored in prevActionState)
    auto prevIt = prevActionState.find(action);
    bool previous = (prevIt != prevActionState.end()) ? prevIt->second : false;
    
    // Just pressed = currently pressed AND was NOT pressed last frame
    return current && !previous;
}

bool InputConfig::isActionJustReleased(GameAction action) const {
    bool current = isActionPressed(action);
    auto prevIt = prevActionState.find(action);
    bool previous = (prevIt != prevActionState.end()) ? prevIt->second : false;
    return !current && previous;
}

bool InputConfig::isKeyPressed(SDL_Keycode key) const {
    auto it = keyState.find(key);
    return it != keyState.end() && it->second;
}

bool InputConfig::isKeyJustPressed(SDL_Keycode key) const {
    auto curIt = keyState.find(key);
    auto prevIt = prevKeyState.find(key);
    bool current = (curIt != keyState.end()) ? curIt->second : false;
    bool previous = (prevIt != prevKeyState.end()) ? prevIt->second : false;
    return current && !previous;
}

bool InputConfig::isGamepadButtonPressed(int button) const {
    auto it = buttonState.find(button);
    return it != buttonState.end() && it->second;
}

bool InputConfig::isGamepadButtonJustPressed(int button) const {
    auto curIt = buttonState.find(button);
    auto prevIt = prevButtonState.find(button);
    bool current = (curIt != buttonState.end()) ? curIt->second : false;
    bool previous = (prevIt != prevButtonState.end()) ? prevIt->second : false;
    return current && !previous;
}

float InputConfig::getGamepadAxis(int axis) const {
    auto it = axisState.find(axis);
    return (it != axisState.end()) ? it->second : 0.0f;
}

int InputConfig::getGamepadHat(int hat) const {
    (void)hat;  // Only support hat 0 for now
    return hatState;
}

void InputConfig::openGamepad(int deviceIndex) {
    if (gamepad) {
        closeGamepad();
    }
    
    gamepad = SDL_GameControllerOpen(deviceIndex);
    if (gamepad) {
        joystick = SDL_GameControllerGetJoystick(gamepad);
        DEBUG_PRINT("Gamepad connected: %s", getGamepadName().c_str());
    }
}

void InputConfig::closeGamepad() {
    if (gamepad) {
        SDL_GameControllerClose(gamepad);
        gamepad = nullptr;
        joystick = nullptr;
        buttonState.clear();
        axisState.clear();
        hatState = 0;
        DEBUG_PRINT("Gamepad disconnected");
    }
}

std::string InputConfig::getGamepadName() const {
    if (gamepad) {
        const char* name = SDL_GameControllerName(gamepad);
        return name ? name : "Unknown Controller";
    }
    return "No Controller";
}

InputBinding InputConfig::waitForKeyboard(int timeoutMs) {
    Uint32 startTime = SDL_GetTicks();
    
    while (SDL_GetTicks() - startTime < (Uint32)timeoutMs) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN) {
                // Ignore modifier keys by themselves
                SDL_Keycode key = event.key.keysym.sym;
                if (key == SDLK_LSHIFT || key == SDLK_RSHIFT ||
                    key == SDLK_LCTRL || key == SDLK_RCTRL ||
                    key == SDLK_LALT || key == SDLK_RALT) {
                    continue;
                }
                return {InputType::KEYBOARD_KEY, key};
            }
            if (event.type == SDL_QUIT) {
                return {};
            }
        }
        SDL_Delay(10);
    }
    
    return {};  // Timeout
}

InputBinding InputConfig::waitForGamepad(int timeoutMs) {
    if (!gamepad) return {};
    
    // For non-blocking mode (timeoutMs == 0), just check current state
    if (timeoutMs == 0) {
        // Check all buttons for any that are currently pressed
        for (int btn = 0; btn < SDL_CONTROLLER_BUTTON_MAX; btn++) {
            if (SDL_GameControllerGetButton(gamepad, static_cast<SDL_GameControllerButton>(btn))) {
                return {InputType::GAMEPAD_BUTTON, btn};
            }
        }
        
        // Check all axes for significant movement
        for (int axis = 0; axis < SDL_CONTROLLER_AXIS_MAX; axis++) {
            float value = SDL_GameControllerGetAxis(gamepad, static_cast<SDL_GameControllerAxis>(axis)) / 32767.0f;
            if (value > 0.5f) {
                return {InputType::GAMEPAD_AXIS_POS, axis};
            } else if (value < -0.5f) {
                return {InputType::GAMEPAD_AXIS_NEG, axis};
            }
        }
        
        // Check hat/dpad via joystick API if available
        if (joystick && SDL_JoystickNumHats(joystick) > 0) {
            int hat = SDL_JoystickGetHat(joystick, 0);
            if (hat != SDL_HAT_CENTERED) {
                return {InputType::GAMEPAD_HAT, hat};
            }
        }
        
        return {};  // No input detected
    }
    
    // Blocking mode with timeout
    Uint32 startTime = SDL_GetTicks();
    
    while (SDL_GetTicks() - startTime < (Uint32)timeoutMs) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_CONTROLLERBUTTONDOWN) {
                return {InputType::GAMEPAD_BUTTON, event.cbutton.button};
            }
            if (event.type == SDL_CONTROLLERAXISMOTION) {
                float value = event.caxis.value / 32767.0f;
                if (value > 0.5f) {
                    return {InputType::GAMEPAD_AXIS_POS, event.caxis.axis};
                } else if (value < -0.5f) {
                    return {InputType::GAMEPAD_AXIS_NEG, event.caxis.axis};
                }
            }
            if (event.type == SDL_JOYHATMOTION && event.jhat.value != 0) {
                return {InputType::GAMEPAD_HAT, event.jhat.value};
            }
            if (event.type == SDL_QUIT) {
                return {};
            }
        }
        SDL_Delay(10);
    }
    
    return {};  // Timeout
}

std::string InputConfig::bindingToString(const InputBinding& binding) {
    switch (binding.type) {
        case InputType::KEYBOARD_KEY:
            return keyToString(static_cast<SDL_Keycode>(binding.code));
        case InputType::GAMEPAD_BUTTON:
            return std::string("BUTTON_") + SDL_GameControllerGetStringForButton(
                static_cast<SDL_GameControllerButton>(binding.code));
        case InputType::GAMEPAD_AXIS_POS:
            return std::string("AXIS_") + SDL_GameControllerGetStringForAxis(
                static_cast<SDL_GameControllerAxis>(binding.code)) + "_POS";
        case InputType::GAMEPAD_AXIS_NEG:
            return std::string("AXIS_") + SDL_GameControllerGetStringForAxis(
                static_cast<SDL_GameControllerAxis>(binding.code)) + "_NEG";
        case InputType::GAMEPAD_HAT:
            switch (binding.code) {
                case SDL_HAT_UP: return "HAT_UP";
                case SDL_HAT_DOWN: return "HAT_DOWN";
                case SDL_HAT_LEFT: return "HAT_LEFT";
                case SDL_HAT_RIGHT: return "HAT_RIGHT";
                default: return "HAT_UNKNOWN";
            }
        default:
            return "NONE";
    }
}

InputBinding InputConfig::stringToBinding(const std::string& str) {
    if (str.empty() || str == "NONE") return {};
    
    // Check for button
    if (str.substr(0, 7) == "BUTTON_") {
        std::string btnName = str.substr(7);
        // Convert to lowercase for SDL
        std::transform(btnName.begin(), btnName.end(), btnName.begin(), ::tolower);
        SDL_GameControllerButton btn = SDL_GameControllerGetButtonFromString(btnName.c_str());
        if (btn != SDL_CONTROLLER_BUTTON_INVALID) {
            return {InputType::GAMEPAD_BUTTON, btn};
        }
    }
    
    // Check for axis
    if (str.substr(0, 5) == "AXIS_") {
        size_t lastUnderscore = str.rfind('_');
        if (lastUnderscore != std::string::npos && lastUnderscore > 5) {
            std::string axisName = str.substr(5, lastUnderscore - 5);
            std::string direction = str.substr(lastUnderscore + 1);
            
            // Convert to lowercase for SDL
            std::transform(axisName.begin(), axisName.end(), axisName.begin(), ::tolower);
            SDL_GameControllerAxis axis = SDL_GameControllerGetAxisFromString(axisName.c_str());
            
            if (axis != SDL_CONTROLLER_AXIS_INVALID) {
                if (direction == "POS") {
                    return {InputType::GAMEPAD_AXIS_POS, axis};
                } else if (direction == "NEG") {
                    return {InputType::GAMEPAD_AXIS_NEG, axis};
                }
            }
        }
    }
    
    // Check for hat/dpad
    if (str.substr(0, 4) == "HAT_") {
        std::string dir = str.substr(4);
        if (dir == "UP") return {InputType::GAMEPAD_HAT, SDL_HAT_UP};
        if (dir == "DOWN") return {InputType::GAMEPAD_HAT, SDL_HAT_DOWN};
        if (dir == "LEFT") return {InputType::GAMEPAD_HAT, SDL_HAT_LEFT};
        if (dir == "RIGHT") return {InputType::GAMEPAD_HAT, SDL_HAT_RIGHT};
    }
    
    // Assume it's a keyboard key
    SDL_Keycode key = stringToKey(str);
    if (key != SDLK_UNKNOWN) {
        return {InputType::KEYBOARD_KEY, key};
    }
    
    return {};
}

std::string InputConfig::actionToString(GameAction action) {
    switch (action) {
        case GameAction::MOVE_LEFT: return "move_left";
        case GameAction::MOVE_RIGHT: return "move_right";
        case GameAction::MOVE_UP: return "move_up";
        case GameAction::MOVE_DOWN: return "move_down";
        case GameAction::JUMP: return "jump";
        case GameAction::FIRE: return "fire";
        case GameAction::PAUSE: return "pause";
        case GameAction::MENU_SELECT: return "menu_select";
        case GameAction::MENU_BACK: return "menu_back";
        default: return "unknown";
    }
}

GameAction InputConfig::stringToAction(const std::string& str) {
    if (str == "move_left") return GameAction::MOVE_LEFT;
    if (str == "move_right") return GameAction::MOVE_RIGHT;
    if (str == "move_up") return GameAction::MOVE_UP;
    if (str == "move_down") return GameAction::MOVE_DOWN;
    if (str == "jump") return GameAction::JUMP;
    if (str == "fire") return GameAction::FIRE;
    if (str == "pause") return GameAction::PAUSE;
    if (str == "menu_select") return GameAction::MENU_SELECT;
    if (str == "menu_back") return GameAction::MENU_BACK;
    return GameAction::COUNT;  // Invalid
}

std::string InputConfig::getActionDisplayName(GameAction action) {
    switch (action) {
        case GameAction::MOVE_LEFT: return "Move Left";
        case GameAction::MOVE_RIGHT: return "Move Right";
        case GameAction::MOVE_UP: return "Move Up";
        case GameAction::MOVE_DOWN: return "Move Down";
        case GameAction::JUMP: return "Jump";
        case GameAction::FIRE: return "Fire/Run";
        case GameAction::PAUSE: return "Pause";
        case GameAction::MENU_SELECT: return "Menu Select";
        case GameAction::MENU_BACK: return "Menu Back";
        default: return "Unknown";
    }
}

std::string InputConfig::keyToString(SDL_Keycode key) {
    const char* name = SDL_GetKeyName(key);
    if (name && name[0] != '\0') {
        return name;
    }
    return "UNKNOWN";
}

SDL_Keycode InputConfig::stringToKey(const std::string& str) {
    return SDL_GetKeyFromName(str.c_str());
}

// Soundfont functions

std::string InputConfig::getSoundfontPath() {
    return "resources/soundfonts/";
}

std::string InputConfig::getSoundfontForTrack(int trackIndex) const {
    auto it = trackSoundfonts.find(trackIndex);
    if (it != trackSoundfonts.end() && !it->second.empty()) {
        return it->second;
    }
    return defaultSoundfont;  // Return default if no override
}

void InputConfig::setSoundfontForTrack(int trackIndex, const std::string& sf) {
    if (sf.empty() || sf == defaultSoundfont) {
        // Remove override if setting to empty or default
        trackSoundfonts.erase(trackIndex);
    } else {
        trackSoundfonts[trackIndex] = sf;
    }
}

std::vector<std::string> InputConfig::getAvailableSoundfonts() const {
    std::vector<std::string> soundfonts;
    soundfonts.push_back("");  // Empty = system default
    
    std::string sfPath = getSoundfontPath();
    
    try {
#ifdef _WIN32
        // Use Windows API for directory enumeration to avoid std::filesystem issues with MinGW
        WIN32_FIND_DATAA findData;
        std::string searchPath = sfPath + "\\*.sf2";
        HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);
        
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    soundfonts.push_back(findData.cFileName);
                }
            } while (FindNextFileA(hFind, &findData));
            FindClose(hFind);
        }
#else
        if (std::filesystem::exists(sfPath) && std::filesystem::is_directory(sfPath)) {
            for (const auto& entry : std::filesystem::directory_iterator(sfPath)) {
                if (entry.is_regular_file()) {
                    std::string filename = entry.path().filename().string();
                    // Only include .sf2 files
                    if (filename.size() > 4) {
                        std::string ext = filename.substr(filename.size() - 4);
                        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                        if (ext == ".sf2") {
                            soundfonts.push_back(filename);
                        }
                    }
                }
            }
        }
#endif
    } catch (const std::exception& e) {
        std::cerr << "[CONFIG] Error scanning soundfonts directory: " << e.what() << std::endl;
    }
    
    // Sort alphabetically (but keep empty string first)
    if (soundfonts.size() > 1) {
        std::sort(soundfonts.begin() + 1, soundfonts.end());
    }
    
    return soundfonts;
}

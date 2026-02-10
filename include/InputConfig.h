/**
 * @file InputConfig.h
 * @brief Input configuration.
 * @ingroup core
 * 
 * InputConfig manages keyboard and gamepad bindings
 * with save/load support.
 */
#pragma once
#include "Common.h"
#include <string>
#include <map>
#include <vector>
#include <functional>

/**
 * InputConfig - Manages keyboard and gamepad input mapping
 * 
 * Features:
 * - Load/save configuration from user's home directory
 * - Support keyboard remapping
 * - Support gamepad remapping (buttons, axes, D-pad)
 * - Default bindings if no config exists
 * - Manual and auto-config modes for gamepad
 */

// Game actions that can be bound
enum class GameAction {
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_UP,
    MOVE_DOWN,
    JUMP,
    FIRE,       // Also "run/speed"
    PAUSE,
    MENU_SELECT,
    MENU_BACK,
    COUNT       // Number of actions
};

// Input source types
enum class InputType {
    NONE,
    KEYBOARD_KEY,
    GAMEPAD_BUTTON,
    GAMEPAD_AXIS_POS,   // Positive axis direction (right, down)
    GAMEPAD_AXIS_NEG,   // Negative axis direction (left, up)
    GAMEPAD_HAT         // D-pad
};

// Binding structure
struct InputBinding {
    InputType type = InputType::NONE;
    int code = 0;       // SDL keycode, button index, axis index, or hat direction
    
    bool operator==(const InputBinding& other) const {
        return type == other.type && code == other.code;
    }
    
    bool isValid() const { return type != InputType::NONE; }
};

class InputConfig {
public:
    static InputConfig& getInstance();
    
    // Initialization
    bool init(bool useDefaults = false);
    void cleanup();
    
    // Configuration file operations
    bool loadConfig();
    bool saveConfig();
    void resetToDefaults();
    
    // Get config file path
    std::string getConfigPath() const;
    
    // Binding management
    void setKeyboardBinding(GameAction action, SDL_Keycode key);
    void setGamepadBinding(GameAction action, InputBinding binding);
    InputBinding getKeyboardBinding(GameAction action) const;
    InputBinding getGamepadBinding(GameAction action) const;
    
    // Input state checking (call after processing SDL events)
    bool isActionPressed(GameAction action) const;
    bool isActionJustPressed(GameAction action) const;
    bool isActionJustReleased(GameAction action) const;
    
    // Raw input state (for menu navigation)
    bool isKeyPressed(SDL_Keycode key) const;
    bool isKeyJustPressed(SDL_Keycode key) const;
    bool isGamepadButtonPressed(int button) const;
    bool isGamepadButtonJustPressed(int button) const;
    float getGamepadAxis(int axis) const;
    int getGamepadHat(int hat = 0) const;
    
    // Event processing (call from Game::handleEvents)
    void processEvent(const SDL_Event& event);
    void updatePreviousState();  // Call at end of frame
    
    // Gamepad management
    bool hasGamepad() const { return gamepad != nullptr; }
    std::string getGamepadName() const;
    void openGamepad(int deviceIndex = 0);
    void closeGamepad();
    
    // For config UI - wait for next input
    InputBinding waitForKeyboard(int timeoutMs = 5000);
    InputBinding waitForGamepad(int timeoutMs = 5000);
    
    // Convert binding to/from string for config file
    static std::string bindingToString(const InputBinding& binding);
    static InputBinding stringToBinding(const std::string& str);
    static std::string actionToString(GameAction action);
    static GameAction stringToAction(const std::string& str);
    static std::string keyToString(SDL_Keycode key);
    static SDL_Keycode stringToKey(const std::string& str);
    
    // Get display name for action
    static std::string getActionDisplayName(GameAction action);
    
    // Axis deadzone
    static constexpr float AXIS_DEADZONE = 0.3f;
    
    // Display settings
    bool isFullscreen() const { return fullscreen; }
    void setFullscreen(bool fs) { fullscreen = fs; }
    
    // Audio settings
    int getMusicVolume() const { return musicVolume; }
    void setMusicVolume(int vol) { musicVolume = (vol < 0 ? 0 : (vol > 128 ? 128 : vol)); }
    int getSfxVolume() const { return sfxVolume; }
    void setSfxVolume(int vol) { sfxVolume = (vol < 0 ? 0 : (vol > 128 ? 128 : vol)); }
    
    // Soundfont settings
    std::string getDefaultSoundfont() const { return defaultSoundfont; }
    void setDefaultSoundfont(const std::string& sf) { defaultSoundfont = sf; }
    std::string getSoundfontForTrack(int trackIndex) const;
    void setSoundfontForTrack(int trackIndex, const std::string& sf);
    std::vector<std::string> getAvailableSoundfonts() const;
    static std::string getSoundfontPath();  // Get path to soundfonts directory
    
private:
    InputConfig() = default;
    ~InputConfig() = default;
    InputConfig(const InputConfig&) = delete;
    InputConfig& operator=(const InputConfig&) = delete;
    
    void setDefaults();
    
    // Current bindings
    std::map<GameAction, InputBinding> keyboardBindings;
    std::map<GameAction, InputBinding> gamepadBindings;
    
    // Input state
    std::map<SDL_Keycode, bool> keyState;
    std::map<SDL_Keycode, bool> prevKeyState;
    std::map<int, bool> buttonState;
    std::map<int, bool> prevButtonState;
    std::map<int, float> axisState;
    int hatState = 0;
    int prevHatState = 0;
    
    // Action state (derived from bindings + input state)
    mutable std::map<GameAction, bool> actionState;
    mutable std::map<GameAction, bool> prevActionState;
    
    // Gamepad
    SDL_GameController* gamepad = nullptr;
    SDL_Joystick* joystick = nullptr;
    
    // Display settings
    bool fullscreen = false;
    
    // Audio settings
    int musicVolume = 96;   // Default 75%
    int sfxVolume = 64;     // Default 50%
    
    // Soundfont settings (empty string = use default)
    std::string defaultSoundfont;  // Default soundfont for all tracks
    std::map<int, std::string> trackSoundfonts;  // Per-track soundfont overrides
    
    bool initialized = false;
    bool useDefaultsOnly = false;
};

// Convenience macros for checking input
// Note: Cannot use "INPUT" as it conflicts with Windows headers (winuser.h)
#define INPUTCFG InputConfig::getInstance()

/**
 * @file Art.h
 * @brief Resource management.
 * @ingroup core
 * 
 * Art loads and manages all game resources:
 * sprite sheets, sounds, music, fonts.
 * 
 * Resources can be overridden by placing files in the user data directory:
 *   Linux: ~/.local/share/infinitetux/
 *   Windows: %APPDATA%/infinitetux/
 */
#pragma once
#include "Common.h"
#include <array>

// Sound sample indices
enum SampleIndex {
    SAMPLE_BREAK_BLOCK = 0,
    SAMPLE_GET_COIN = 1,
    SAMPLE_MARIO_JUMP = 2,
    SAMPLE_MARIO_STOMP = 3,
    SAMPLE_MARIO_KICK = 4,
    SAMPLE_MARIO_POWER_UP = 5,
    SAMPLE_MARIO_POWER_DOWN = 6,
    SAMPLE_MARIO_DEATH = 7,
    SAMPLE_ITEM_SPROUT = 8,
    SAMPLE_CANNON_FIRE = 9,
    SAMPLE_SHELL_BUMP = 10,
    SAMPLE_LEVEL_EXIT = 11,
    SAMPLE_MARIO_1UP = 12,
    SAMPLE_MARIO_FIREBALL = 13,
    SAMPLE_LOW_TIME = 14,
    SAMPLE_COUNT = 15
};

// Music indices
enum MusicIndex {
    MUSIC_MAP = 0,
    MUSIC_OVERWORLD = 1,
    MUSIC_UNDERGROUND = 2,
    MUSIC_CASTLE = 3,
    MUSIC_TITLE = 4,
    MUSIC_COUNT = 5
};

class Art {
public:
    static bool init(SDL_Renderer* renderer, const std::string& resourcePath);
    static void cleanup();
    
    // Get the user data directory (XDG compliant)
    static std::string getUserDataDir();
    
    // Resolve a resource path - checks user directory first, then system
    static std::string resolveResource(const std::string& relativePath);
    
    // Sprite sheets (2D arrays of textures)
    static std::vector<std::vector<SDL_Texture*>> mario;
    static std::vector<std::vector<SDL_Texture*>> smallMario;
    static std::vector<std::vector<SDL_Texture*>> fireMario;
    static std::vector<std::vector<SDL_Texture*>> enemies;
    static std::vector<std::vector<SDL_Texture*>> items;
    static std::vector<std::vector<SDL_Texture*>> level;
    static std::vector<std::vector<SDL_Texture*>> particles;
    static std::vector<std::vector<SDL_Texture*>> font;
    static std::vector<std::vector<SDL_Texture*>> bg;
    static std::vector<std::vector<SDL_Texture*>> map;
    static std::vector<std::vector<SDL_Texture*>> endScene;
    static std::vector<std::vector<SDL_Texture*>> gameOver;
    
    // Single images
    static SDL_Texture* logo;
    static SDL_Texture* titleScreen;
    
    // Sound effects
    static std::array<Mix_Chunk*, SAMPLE_COUNT> samples;
    
    // Music
    static std::array<Mix_Music*, MUSIC_COUNT> music;
    static int currentMusic;
    
    // Renderer reference
    static SDL_Renderer* renderer;
    static std::string resourcePath;
    static std::string userDataDir;
    
    // Helper functions
    static void playSound(int sampleIndex);
    static void startMusic(int musicIndex, bool forceRestart = false);
    static void stopMusic();
    static void cycleMidiSynth();  // Cycle through MIDI synth options
    static void adjustMusicVolume(int delta);  // Adjust music volume (+/- delta)
    static int getMusicVolume();  // Get current music volume (0-128)
    static void adjustSfxVolume(int delta);  // Adjust SFX volume (+/- delta)
    static int getSfxVolume();  // Get current SFX volume (0-128)
    static void initVolumeFromConfig();  // Load volume settings from config
    
    // Draw text
    static void drawString(const std::string& text, int x, int y, int color);
    
private:
    static SDL_Surface* loadImage(const std::string& path);
    static std::vector<std::vector<SDL_Texture*>> cutImage(const std::string& path, int xSize, int ySize);
    static SDL_Texture* loadTexture(const std::string& path);
    
public:
    // File existence check (public for use by helper functions)
    static bool fileExists(const std::string& path);
};

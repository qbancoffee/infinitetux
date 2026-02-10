/**
 * @file Art.cpp
 * @brief Resource loading implementation.
 * 
 * Resources are loaded with the following priority:
 * 1. User data directory (~/.local/share/infinitetux/ on Linux)
 * 2. System resource directory (/usr/share/games/infinitetux/resources/)
 * 
 * This allows users to override any resource by placing a file with the
 * same name in their user data directory.
 */
#include "Art.h"
#include "InputConfig.h"
#include <iostream>
#include <fstream>
#include <array>
#include <sys/stat.h>
#include <cerrno>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include <unistd.h>
#include <pwd.h>
#endif

// Static member initialization
std::vector<std::vector<SDL_Texture*>> Art::mario;
std::vector<std::vector<SDL_Texture*>> Art::smallMario;
std::vector<std::vector<SDL_Texture*>> Art::fireMario;
std::vector<std::vector<SDL_Texture*>> Art::enemies;
std::vector<std::vector<SDL_Texture*>> Art::items;
std::vector<std::vector<SDL_Texture*>> Art::level;
std::vector<std::vector<SDL_Texture*>> Art::particles;
std::vector<std::vector<SDL_Texture*>> Art::font;
std::vector<std::vector<SDL_Texture*>> Art::bg;
std::vector<std::vector<SDL_Texture*>> Art::map;
std::vector<std::vector<SDL_Texture*>> Art::endScene;
std::vector<std::vector<SDL_Texture*>> Art::gameOver;

SDL_Texture* Art::logo = nullptr;
SDL_Texture* Art::titleScreen = nullptr;

std::array<Mix_Chunk*, SAMPLE_COUNT> Art::samples = {};
std::array<Mix_Music*, MUSIC_COUNT> Art::music = {};
int Art::currentMusic = -1;

SDL_Renderer* Art::renderer = nullptr;
std::string Art::resourcePath;
std::string Art::userDataDir;

// Check if a file exists
bool Art::fileExists(const std::string& path) {
    std::ifstream f(path);
    return f.good();
}

// Get the user data directory following XDG Base Directory Specification
std::string Art::getUserDataDir() {
    std::string dataDir;
    
#ifdef _WIN32
    // Windows: Use APPDATA
    char* appData = getenv("APPDATA");
    if (appData) {
        dataDir = std::string(appData) + "\\infinitetux\\";
    } else {
        dataDir = ".\\infinitetux-data\\";
    }
#else
    // Linux/Unix: Follow XDG Base Directory Specification
    // XDG_DATA_HOME defaults to ~/.local/share
    const char* xdgDataHome = getenv("XDG_DATA_HOME");
    if (xdgDataHome && strlen(xdgDataHome) > 0) {
        dataDir = std::string(xdgDataHome) + "/infinitetux/";
    } else {
        // Default to ~/.local/share/infinitetux
        const char* home = getenv("HOME");
        if (!home) {
            struct passwd* pw = getpwuid(getuid());
            if (pw) {
                home = pw->pw_dir;
            }
        }
        if (home) {
            dataDir = std::string(home) + "/.local/share/infinitetux/";
        } else {
            dataDir = "./infinitetux-data/";
        }
    }
#endif
    
    return dataDir;
}

// Resolve a resource path - check user directory first, then system
std::string Art::resolveResource(const std::string& relativePath) {
    // First, check user data directory
    if (!userDataDir.empty()) {
        std::string userPath = userDataDir + relativePath;
        if (fileExists(userPath)) {
            DEBUG_PRINT("Using user override: %s", userPath.c_str());
            return userPath;
        }
    }
    
    // Fall back to system resource path
    return resourcePath + relativePath;
}

// Forward declaration
static void createUserDataStructure(const std::string& userDir);

bool Art::init(SDL_Renderer* rend, const std::string& resPath) {
    renderer = rend;
    resourcePath = resPath;
    
    // Initialize user data directory
    userDataDir = getUserDataDir();
    DEBUG_PRINT("User data directory: %s", userDataDir.c_str());
    DEBUG_PRINT("System resource path: %s", resourcePath.c_str());
    
    // Create user data directory structure with README (for custom resources)
    createUserDataStructure(userDataDir);
    
    try {
        // Load sprite sheets (with user override support)
        mario = cutImage(resolveResource("mariosheet.png"), 32, 32);
        smallMario = cutImage(resolveResource("smallmariosheet.png"), 16, 16);
        fireMario = cutImage(resolveResource("firemariosheet.png"), 32, 32);
        enemies = cutImage(resolveResource("enemysheet.png"), 16, 32);
        items = cutImage(resolveResource("itemsheet.png"), 16, 16);
        level = cutImage(resolveResource("mapsheet.png"), 16, 16);
        map = cutImage(resolveResource("worldmap.png"), 16, 16);
        particles = cutImage(resolveResource("particlesheet.png"), 8, 8);
        bg = cutImage(resolveResource("bgsheet.png"), 32, 32);
        font = cutImage(resolveResource("font.gif"), 8, 8);
        endScene = cutImage(resolveResource("endscene.gif"), 96, 96);
        gameOver = cutImage(resolveResource("gameovergost.gif"), 96, 64);
        
        // Load single images
        logo = loadTexture(resolveResource("logo.gif"));
        titleScreen = loadTexture(resolveResource("title.gif"));
        
        // Load sounds
        samples[SAMPLE_BREAK_BLOCK] = Mix_LoadWAV(resolveResource("snd/breakblock.wav").c_str());
        samples[SAMPLE_GET_COIN] = Mix_LoadWAV(resolveResource("snd/coin.wav").c_str());
        samples[SAMPLE_MARIO_JUMP] = Mix_LoadWAV(resolveResource("snd/jump.wav").c_str());
        samples[SAMPLE_MARIO_STOMP] = Mix_LoadWAV(resolveResource("snd/stomp.wav").c_str());
        samples[SAMPLE_MARIO_KICK] = Mix_LoadWAV(resolveResource("snd/kick.wav").c_str());
        samples[SAMPLE_MARIO_POWER_UP] = Mix_LoadWAV(resolveResource("snd/powerup.wav").c_str());
        samples[SAMPLE_MARIO_POWER_DOWN] = Mix_LoadWAV(resolveResource("snd/powerdown.wav").c_str());
        samples[SAMPLE_MARIO_DEATH] = Mix_LoadWAV(resolveResource("snd/death.wav").c_str());
        samples[SAMPLE_ITEM_SPROUT] = Mix_LoadWAV(resolveResource("snd/sprout.wav").c_str());
        samples[SAMPLE_CANNON_FIRE] = Mix_LoadWAV(resolveResource("snd/cannon.wav").c_str());
        samples[SAMPLE_SHELL_BUMP] = Mix_LoadWAV(resolveResource("snd/bump.wav").c_str());
        samples[SAMPLE_LEVEL_EXIT] = Mix_LoadWAV(resolveResource("snd/exit.wav").c_str());
        samples[SAMPLE_MARIO_1UP] = Mix_LoadWAV(resolveResource("snd/1-up.wav").c_str());
        samples[SAMPLE_MARIO_FIREBALL] = Mix_LoadWAV(resolveResource("snd/fireball.wav").c_str());
        samples[SAMPLE_LOW_TIME] = Mix_LoadWAV(resolveResource("snd/lowtime.wav").c_str());
        
        // Note: Music is loaded on-demand in startMusic() to support soundfont switching
        // Music files are also resolved through resolveResource() there
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading resources: " << e.what() << std::endl;
        return false;
    }
    
    return true;
}

// Create a directory (cross-platform)
static bool createDirectory(const std::string& path) {
#ifdef _WIN32
    return CreateDirectoryA(path.c_str(), NULL) || GetLastError() == ERROR_ALREADY_EXISTS;
#else
    return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
#endif
}

// Create the user data directory structure with README
static void createUserDataStructure(const std::string& userDir) {
    // Create main directory
    createDirectory(userDir);
    
    // Create subdirectories
    createDirectory(userDir + "snd");
    createDirectory(userDir + "mus");
    createDirectory(userDir + "soundfonts");
    
    // Create README if it doesn't exist
    std::string readmePath = userDir + "README.txt";
    if (!Art::fileExists(readmePath)) {
        std::ofstream readme(readmePath);
        if (readme.is_open()) {
            readme << "INFINITE TUX - Custom Resources Directory\n";
            readme << "==========================================\n\n";
            readme << "Place custom resource files in this directory to override the default game assets.\n";
            readme << "Files placed here take priority over the system-installed resources.\n\n";
            readme << "DIRECTORY STRUCTURE:\n";
            readme << "--------------------\n";
            readme << userDir << "\n";
            readme << "├── snd/          - Sound effects (WAV format)\n";
            readme << "├── mus/          - Music files (MIDI format)\n";
            readme << "├── soundfonts/   - Custom soundfonts (SF2 format)\n";
            readme << "└── (image files) - Sprite sheets and images\n\n";
            readme << "IMAGE FILES (place in main directory):\n";
            readme << "---------------------------------------\n";
            readme << "mariosheet.png      - Big Mario sprites (32x32 per frame)\n";
            readme << "smallmariosheet.png - Small Mario sprites (16x16 per frame)\n";
            readme << "firemariosheet.png  - Fire Mario sprites (32x32 per frame)\n";
            readme << "enemysheet.png      - Enemy sprites (16x32 per frame)\n";
            readme << "itemsheet.png       - Item sprites: coins, mushrooms, flowers (16x16 per frame)\n";
            readme << "mapsheet.png        - Level tile graphics (16x16 per tile)\n";
            readme << "worldmap.png        - World map tiles (16x16 per tile)\n";
            readme << "particlesheet.png   - Particle effects (8x8 per frame)\n";
            readme << "bgsheet.png         - Background tiles (32x32 per tile)\n";
            readme << "font.gif            - Font characters (8x8 per character)\n";
            readme << "endscene.gif        - End scene graphic (96x96)\n";
            readme << "gameovergost.gif    - Game over ghost graphic (96x64)\n";
            readme << "logo.gif            - Game logo\n";
            readme << "title.gif           - Title screen background\n\n";
            readme << "DATA FILES (place in main directory):\n";
            readme << "--------------------------------------\n";
            readme << "tiles.dat           - Tile behavior data (256 bytes, one per tile type)\n\n";
            readme << "SOUND FILES (place in snd/ subdirectory):\n";
            readme << "------------------------------------------\n";
            readme << "snd/breakblock.wav  - Block breaking sound\n";
            readme << "snd/coin.wav        - Coin collection sound\n";
            readme << "snd/jump.wav        - Mario jump sound\n";
            readme << "snd/stomp.wav       - Enemy stomp sound\n";
            readme << "snd/kick.wav        - Shell kick sound\n";
            readme << "snd/powerup.wav     - Power-up collection sound\n";
            readme << "snd/powerdown.wav   - Power-down (damage) sound\n";
            readme << "snd/death.wav       - Mario death sound\n";
            readme << "snd/sprout.wav      - Item sprouting from block sound\n";
            readme << "snd/cannon.wav      - Bullet Bill cannon sound\n";
            readme << "snd/bump.wav        - Bump/shell hit sound\n";
            readme << "snd/exit.wav        - Level exit sound\n";
            readme << "snd/1-up.wav        - Extra life sound\n";
            readme << "snd/fireball.wav    - Fireball throw sound\n";
            readme << "snd/lowtime.wav     - Low time warning sound\n\n";
            readme << "MUSIC FILES (place in mus/ subdirectory):\n";
            readme << "------------------------------------------\n";
            readme << "mus/smb3map1.mid    - World map music\n";
            readme << "mus/smwovr1.mid     - Overworld level music\n";
            readme << "mus/smb3undr.mid    - Underground level music\n";
            readme << "mus/smwfortress.mid - Castle/fortress level music\n";
            readme << "mus/smwtitle.mid    - Title screen music\n\n";
            readme << "SOUNDFONTS (place in soundfonts/ subdirectory):\n";
            readme << "------------------------------------------------\n";
            readme << "soundfonts/*.sf2    - Custom SoundFont files for MIDI playback\n\n";
            readme << "NOTES:\n";
            readme << "------\n";
            readme << "- All image files should maintain the same dimensions and frame layout\n";
            readme << "  as the originals to ensure proper rendering.\n";
            readme << "- Sound files should be in WAV format (PCM recommended).\n";
            readme << "- Music files should be in MIDI format (.mid).\n";
            readme << "- Run the game with --debug to see which files are being loaded.\n";
            readme << "- Changes take effect on next game launch (no hot-reloading).\n\n";
            readme << "EXAMPLE:\n";
            readme << "--------\n";
            readme << "To replace Mario's sprites, create your own 'mariosheet.png' with the\n";
            readme << "same dimensions and place it in this directory. The game will\n";
            readme << "automatically use your custom file instead of the default.\n";
            readme.close();
            DEBUG_PRINT("Created user data README at %s", readmePath.c_str());
        }
    }
}

void Art::cleanup() {
    // Clean up sprite sheets
    auto cleanupSheet = [](std::vector<std::vector<SDL_Texture*>>& sheet) {
        for (auto& row : sheet) {
            for (auto* tex : row) {
                if (tex) SDL_DestroyTexture(tex);
            }
        }
        sheet.clear();
    };
    
    cleanupSheet(mario);
    cleanupSheet(smallMario);
    cleanupSheet(fireMario);
    cleanupSheet(enemies);
    cleanupSheet(items);
    cleanupSheet(level);
    cleanupSheet(particles);
    cleanupSheet(font);
    cleanupSheet(bg);
    cleanupSheet(map);
    cleanupSheet(endScene);
    cleanupSheet(gameOver);
    
    if (logo) { SDL_DestroyTexture(logo); logo = nullptr; }
    if (titleScreen) { SDL_DestroyTexture(titleScreen); titleScreen = nullptr; }
    
    // Clean up sounds
    for (auto& sample : samples) {
        if (sample) { Mix_FreeChunk(sample); sample = nullptr; }
    }
    
    // Clean up music
    for (auto& mus : music) {
        if (mus) { Mix_FreeMusic(mus); mus = nullptr; }
    }
}

SDL_Surface* Art::loadImage(const std::string& path) {
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        std::cerr << "Failed to load image " << path << ": " << IMG_GetError() << std::endl;
    }
    return surface;
}

SDL_Texture* Art::loadTexture(const std::string& path) {
    SDL_Surface* surface = loadImage(path);
    if (!surface) return nullptr;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    if (!texture) {
        std::cerr << "Failed to create texture from " << path << ": " << SDL_GetError() << std::endl;
    }
    
    return texture;
}

std::vector<std::vector<SDL_Texture*>> Art::cutImage(const std::string& path, int xSize, int ySize) {
    SDL_Surface* source = loadImage(path);
    if (!source) {
        DEBUG_PRINT("Art::cutImage FAILED to load: %s", path.c_str());
        return {};
    }
    
    int xCount = source->w / xSize;
    int yCount = source->h / ySize;
    
    DEBUG_PRINT("Art::cutImage loaded %s: %dx%d pixels -> %dx%d tiles (%dx%d each)", 
                path.c_str(), source->w, source->h, xCount, yCount, xSize, ySize);
    
    std::vector<std::vector<SDL_Texture*>> images(xCount);
    
    // Check if source has alpha channel
    bool hasAlpha = (source->format->Amask != 0);
    
    // Check if source has color key (GIF transparency)
    Uint32 sourceColorKey;
    bool hasColorKey = (SDL_GetColorKey(source, &sourceColorKey) == 0);
    
    for (int x = 0; x < xCount; x++) {
        images[x].resize(yCount);
        for (int y = 0; y < yCount; y++) {
            SDL_Surface* tileSurface = nullptr;
            
            if (hasAlpha) {
                // Source has alpha channel - create RGBA surface and blit directly
                tileSurface = SDL_CreateRGBSurfaceWithFormat(
                    0, xSize, ySize, 32, SDL_PIXELFORMAT_RGBA32);
                
                if (tileSurface) {
                    // Clear to fully transparent
                    SDL_FillRect(tileSurface, nullptr, SDL_MapRGBA(tileSurface->format, 0, 0, 0, 0));
                    
                    // Enable blending on source for proper alpha copy
                    SDL_SetSurfaceBlendMode(source, SDL_BLENDMODE_NONE);
                    
                    SDL_Rect srcRect = {x * xSize, y * ySize, xSize, ySize};
                    SDL_BlitSurface(source, &srcRect, tileSurface, nullptr);
                }
            } else if (hasColorKey || source->format->BytesPerPixel == 1) {
                // Paletted image with color key - handle manually
                tileSurface = SDL_CreateRGBSurfaceWithFormat(
                    0, xSize, ySize, 32, SDL_PIXELFORMAT_RGBA32);
                
                if (tileSurface) {
                    SDL_FillRect(tileSurface, nullptr, SDL_MapRGBA(tileSurface->format, 0, 0, 0, 0));
                    
                    SDL_LockSurface(source);
                    SDL_LockSurface(tileSurface);
                    
                    for (int py = 0; py < ySize; py++) {
                        for (int px = 0; px < xSize; px++) {
                            int srcX = x * xSize + px;
                            int srcY = y * ySize + py;
                            
                            Uint8 r, g, b, a = 255;
                            
                            if (source->format->BytesPerPixel == 1) {
                                // Paletted
                                Uint8* srcPixel = (Uint8*)source->pixels + srcY * source->pitch + srcX;
                                Uint8 index = *srcPixel;
                                SDL_GetRGB(index, source->format, &r, &g, &b);
                                
                                // Check transparency by palette index
                                if (hasColorKey && index == (sourceColorKey & 0xFF)) {
                                    a = 0;
                                }
                            } else {
                                // Get pixel value
                                Uint8* srcPixel = (Uint8*)source->pixels + srcY * source->pitch + srcX * source->format->BytesPerPixel;
                                Uint32 pixel;
                                memcpy(&pixel, srcPixel, source->format->BytesPerPixel);
                                SDL_GetRGB(pixel, source->format, &r, &g, &b);
                                
                                // Check for magenta transparency
                                if (r == 255 && g == 0 && b == 255) {
                                    a = 0;
                                }
                            }
                            
                            Uint32* dstPixel = (Uint32*)((Uint8*)tileSurface->pixels + py * tileSurface->pitch) + px;
                            *dstPixel = SDL_MapRGBA(tileSurface->format, r, g, b, a);
                        }
                    }
                    
                    SDL_UnlockSurface(tileSurface);
                    SDL_UnlockSurface(source);
                }
            } else {
                // No alpha, no color key - use magenta as transparency
                tileSurface = SDL_CreateRGBSurfaceWithFormat(
                    0, xSize, ySize, 32, SDL_PIXELFORMAT_RGBA32);
                
                if (tileSurface) {
                    SDL_Rect srcRect = {x * xSize, y * ySize, xSize, ySize};
                    SDL_BlitSurface(source, &srcRect, tileSurface, nullptr);
                    
                    // Set magenta as color key
                    Uint32 colorKey = SDL_MapRGB(tileSurface->format, 255, 0, 255);
                    SDL_SetColorKey(tileSurface, SDL_TRUE, colorKey);
                }
            }
            
            if (tileSurface) {
                images[x][y] = SDL_CreateTextureFromSurface(renderer, tileSurface);
                if (images[x][y]) {
                    SDL_SetTextureBlendMode(images[x][y], SDL_BLENDMODE_BLEND);
                }
                SDL_FreeSurface(tileSurface);
            }
        }
    }
    
    SDL_FreeSurface(source);
    return images;
}

// Current sound effect volume (0-128)  
static int sfxVolume = 64;  // Default to 50% to balance with quieter music

void Art::playSound(int sampleIndex) {
    if (sampleIndex >= 0 && sampleIndex < SAMPLE_COUNT && samples[sampleIndex]) {
        int channel = Mix_PlayChannel(-1, samples[sampleIndex], 0);
        if (channel >= 0) {
            Mix_Volume(channel, sfxVolume);
        }
    }
}

// Current music volume (0-128)
static int musicVolume = 96;  // Default to 75% to avoid distortion

void Art::initVolumeFromConfig() {
    // Load saved volume settings from InputConfig
    musicVolume = INPUTCFG.getMusicVolume();
    sfxVolume = INPUTCFG.getSfxVolume();
    Mix_VolumeMusic(musicVolume);
    DEBUG_PRINT("Loaded volumes - Music: %d%%, SFX: %d%%", 
                (musicVolume * 100 / MIX_MAX_VOLUME), 
                (sfxVolume * 100 / MIX_MAX_VOLUME));
}

// Helper function to build full soundfont path
static std::string buildSoundfontPath(const std::string& soundfont) {
    if (soundfont.empty()) {
        return "";  // Use system default
    }
    return InputConfig::getSoundfontPath() + soundfont;
}

// Apply soundfont for a specific track
static void applySoundfontForTrack(int musicIndex) {
    std::string soundfont = INPUTCFG.getSoundfontForTrack(musicIndex);
    
    if (soundfont.empty()) {
        // No custom soundfont - use system defaults
#ifdef _WIN32
        Mix_SetSoundFonts("C:\\soundfonts\\FluidR3_GM.sf2");
#else
        Mix_SetSoundFonts("/usr/share/sounds/sf2/FluidR3_GM.sf2");
#endif
    } else {
        // Use custom soundfont from resources/soundfonts/
        std::string fullPath = buildSoundfontPath(soundfont);
        
        // Check if file exists before trying to use it
        std::ifstream testFile(fullPath);
        if (testFile.good()) {
            testFile.close();
            Mix_SetSoundFonts(fullPath.c_str());
        } else {
            std::cerr << "[AUDIO] Soundfont not found: " << fullPath << ", using system default" << std::endl;
#ifdef _WIN32
            Mix_SetSoundFonts("C:\\soundfonts\\FluidR3_GM.sf2");
#else
            Mix_SetSoundFonts("/usr/share/sounds/sf2/FluidR3_GM.sf2");
#endif
        }
    }
}

// Get music file path for track index (with user override support)
static std::string getMusicPath(int musicIndex) {
    switch (musicIndex) {
        case MUSIC_MAP: return Art::resolveResource("mus/smb3map1.mid");
        case MUSIC_OVERWORLD: return Art::resolveResource("mus/smwovr1.mid");
        case MUSIC_UNDERGROUND: return Art::resolveResource("mus/smb3undr.mid");
        case MUSIC_CASTLE: return Art::resolveResource("mus/smwfortress.mid");
        case MUSIC_TITLE: return Art::resolveResource("mus/smwtitle.mid");
        default: return "";
    }
}

// Flag to prevent rapid music restarts which can crash FluidSynth
static bool musicLoadInProgress = false;

// Track which soundfont is currently loaded for each track
static std::array<std::string, MUSIC_COUNT> currentSoundfonts = {};

void Art::startMusic(int musicIndex, bool forceRestart) {
    DEBUG_PRINT("startMusic(%d, forceRestart=%d) called", musicIndex, forceRestart);
    
    // Prevent rapid restarts
    if (musicLoadInProgress) {
        DEBUG_PRINT("Music load in progress, skipping");
        return;
    }
    
    // Validate index first
    if (musicIndex < 0 || musicIndex >= MUSIC_COUNT) {
        currentMusic = -1;
        DEBUG_PRINT("Invalid music index");
        return;
    }
    
    musicLoadInProgress = true;
    
    // Check if soundfont has changed
    DEBUG_PRINT("Getting soundfont for track...");
    std::string newSoundfont = INPUTCFG.getSoundfontForTrack(musicIndex);
    bool soundfontChanged = (currentSoundfonts[musicIndex] != newSoundfont);
    DEBUG_PRINT("Soundfont: '%s', changed=%d", newSoundfont.c_str(), soundfontChanged);
    
    // If music exists, soundfont hasn't changed, AND we're not forcing a restart, just play it
    // When forceRestart is true, we must reload to ensure music starts from the beginning
    // (Mix_RewindMusic doesn't reliably reset MIDI position after Mix_HaltMusic)
    if (music[musicIndex] && !soundfontChanged && !forceRestart) {
        DEBUG_PRINT("Playing cached music...");
        
        // Stop any currently playing music
        if (Mix_PlayingMusic()) {
            Mix_HaltMusic();
        }
        
        // Rewind to beginning if it's a different track
        if (currentMusic != musicIndex) {
            Mix_RewindMusic();
        }
        
        Mix_VolumeMusic(musicVolume);
        if (Mix_PlayMusic(music[musicIndex], -1) == -1) {
            std::cerr << "[AUDIO] Failed to play music: " << Mix_GetError() << std::endl;
            currentMusic = -1;
        } else {
            currentMusic = musicIndex;
        }
        musicLoadInProgress = false;
        DEBUG_PRINT("Cached music playing");
        return;
    }
    
    DEBUG_PRINT("Need to load music...");
    
    // Stop any currently playing music
    if (Mix_PlayingMusic()) {
        DEBUG_PRINT("Halting current music...");
        Mix_HaltMusic();
    }
    
    // Free old music if it exists
    if (music[musicIndex]) {
        DEBUG_PRINT("Freeing old music...");
        Mix_FreeMusic(music[musicIndex]);
        music[musicIndex] = nullptr;
    }
    
    // Apply soundfont for this track (must be done before loading)
    DEBUG_PRINT("Applying soundfont...");
    applySoundfontForTrack(musicIndex);
    currentSoundfonts[musicIndex] = newSoundfont;  // Remember the soundfont
    
    std::string musicPath = getMusicPath(musicIndex);
    DEBUG_PRINT("Music path: %s", musicPath.c_str());
    
    if (musicPath.empty()) {
        currentMusic = -1;
        musicLoadInProgress = false;
        DEBUG_PRINT("Empty music path, aborting");
        return;
    }
    
    // Load the music file
    DEBUG_PRINT("Calling Mix_LoadMUS()...");
    music[musicIndex] = Mix_LoadMUS(musicPath.c_str());
    DEBUG_PRINT("Mix_LoadMUS returned: %s", (music[musicIndex] ? "success" : "null"));
    
    if (music[musicIndex]) {
        // Set music volume before playing to prevent distortion
        Mix_VolumeMusic(musicVolume);
        
        // Play the music
        if (Mix_PlayMusic(music[musicIndex], -1) == -1) {
            std::cerr << "[AUDIO] Failed to play music: " << Mix_GetError() << std::endl;
            Mix_FreeMusic(music[musicIndex]);
            music[musicIndex] = nullptr;
            currentMusic = -1;
        } else {
            currentMusic = musicIndex;
        }
    } else {
        std::cerr << "[AUDIO] Failed to load music: " << musicPath << " - " << Mix_GetError() << std::endl;
        currentMusic = -1;
    }
    
    musicLoadInProgress = false;
}

void Art::stopMusic() {
    if (Mix_PlayingMusic()) {
        Mix_HaltMusic();
    }
    currentMusic = -1;
}

void Art::adjustMusicVolume(int delta) {
    musicVolume += delta;
    if (musicVolume < 0) musicVolume = 0;
    if (musicVolume > MIX_MAX_VOLUME) musicVolume = MIX_MAX_VOLUME;
    Mix_VolumeMusic(musicVolume);
    
    // Save to config for persistence
    INPUTCFG.setMusicVolume(musicVolume);
    INPUTCFG.saveConfig();
    
    DEBUG_PRINT("Music volume: %d%%", (musicVolume * 100 / MIX_MAX_VOLUME));
}

int Art::getMusicVolume() {
    return musicVolume;
}

void Art::adjustSfxVolume(int delta) {
    sfxVolume += delta;
    if (sfxVolume < 0) sfxVolume = 0;
    if (sfxVolume > MIX_MAX_VOLUME) sfxVolume = MIX_MAX_VOLUME;
    
    // Save to config for persistence
    INPUTCFG.setSfxVolume(sfxVolume);
    INPUTCFG.saveConfig();
    
    DEBUG_PRINT("SFX volume: %d%%", (sfxVolume * 100 / MIX_MAX_VOLUME));
}

int Art::getSfxVolume() {
    return sfxVolume;
}

// Static variable to track current MIDI synth setting
static int midiSynthType = 0;  // 0=default, 1=native, 2=timidity

void Art::cycleMidiSynth() {
    midiSynthType = (midiSynthType + 1) % 3;
    
    const char* synthName;
    switch (midiSynthType) {
        case 0:
            synthName = "Default (SDL_mixer auto)";
            // Clear any custom settings, use SDL_mixer default
            Mix_SetSoundFonts(NULL);
            break;
        case 1:
            synthName = "Native MIDI";
            // Try to use native MIDI (Windows/macOS have built-in synths)
            // This hint tells SDL_mixer to prefer native MIDI
            SDL_SetHint("SDL_NATIVE_MUSIC", "1");
            Mix_SetSoundFonts(NULL);
            break;
        case 2:
            synthName = "FluidSynth (if available)";
            // FluidSynth typically sounds better but needs a soundfont
            SDL_SetHint("SDL_NATIVE_MUSIC", "0");
            // Set soundfont path - FluidSynth will use the first one it finds
            // Note: FluidSynth may print errors for paths that don't exist, this is normal
#ifdef _WIN32
            Mix_SetSoundFonts("C:\\soundfonts\\FluidR3_GM.sf2");
#else
            // Most common location on Debian/Ubuntu systems
            Mix_SetSoundFonts("/usr/share/sounds/sf2/FluidR3_GM.sf2");
#endif
            break;
        default:
            synthName = "Default";
            break;
    }
    
    DEBUG_PRINT("MIDI Synth: %s", synthName);
    
    // Reload and restart current music with new synth
    if (currentMusic >= 0 && currentMusic < MUSIC_COUNT) {
        int savedMusic = currentMusic;
        startMusic(savedMusic);  // This will reload with the current soundfont settings
    }
}

void Art::drawString(const std::string& text, int x, int y, int color) {
    if (font.empty() || font[0].empty()) return;
    
    for (size_t i = 0; i < text.length(); i++) {
        char ch = text[i];
        int charIndex = ch - 32;  // ASCII 32 is space, first character
        if (charIndex < 0 || charIndex >= (int)font.size()) continue;
        
        // font[charIndex] is a column of color variants
        // font[charIndex][color] is the specific colored character
        int colorIndex = color;
        if (colorIndex < 0) colorIndex = 0;
        if (colorIndex >= (int)font[charIndex].size()) colorIndex = (int)font[charIndex].size() - 1;
        
        if (font[charIndex][colorIndex] != nullptr) {
            SDL_Rect dst = {x + (int)i * 8, y, 8, 8};
            SDL_RenderCopy(renderer, font[charIndex][colorIndex], nullptr, &dst);
        }
    }
}

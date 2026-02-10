/**
 * main.cpp - Entry point for Infinite Tux
 * 
 * A C++/SDL2 port of Infinite Mario Bros, featuring procedurally generated levels.
 */

#include "Game.h"
#include "Common.h"
#include <iostream>
#include <cstring>
#include <cstdio>

// Global debug flag
bool g_debugMode = false;

// Global test mode flag
bool g_testMode = false;

// Global test mode invincibility (default on in test mode, toggle with `)
bool g_testInvincible = false;

void printHelp(const char* programName) {
    std::cout << "Infinite Tux v1.0.1 - A C++/SDL2 port of Infinite Mario Bros\n";
    std::cout << "\n";
    std::cout << "USAGE: " << programName << " [OPTIONS]\n";
    std::cout << "\n";
    std::cout << "OPTIONS:\n";
    std::cout << "  -h, --help      Show this help message and exit\n";
    std::cout << "  -d, --debug     Enable debug output (spawn messages, collision info, etc.)\n";
    std::cout << "  -t, --test      Enable test mode (see TEST MODE below)\n";
    std::cout << "  --default       Use default input bindings, ignoring config file\n";
    std::cout << "                  (Use this if custom bindings are broken)\n";
    std::cout << "\n";
    std::cout << "GAMEPLAY CONTROLS:\n";
    std::cout << "  Arrow Keys      Move left/right, climb vines, duck (down)\n";
    std::cout << "  A / Z           Jump (hold longer to jump higher)\n";
    std::cout << "  S / X           Run (hold while moving), shoot fireballs,\n";
    std::cout << "                  pick up shells (hold while stomping)\n";
    std::cout << "  Enter           Pause/unpause game\n";
    std::cout << "  Escape          Quit game\n";
    std::cout << "  F9              Cycle MIDI synth (Default/Native/FluidSynth)\n";
    std::cout << "  F10             Cycle scale quality (Nearest/Linear/Best)\n";
    std::cout << "  F11             Toggle fullscreen\n";
    std::cout << "\n";
    std::cout << "MENU CONTROLS:\n";
    std::cout << "  Arrow Keys      Navigate menu options\n";
    std::cout << "  A / Z / Enter   Select option\n";
    std::cout << "\n";
    std::cout << "GAMEPLAY TIPS:\n";
    std::cout << "  - Stomp enemies by landing on them from above\n";
    std::cout << "  - Hit ? blocks from below to get coins and power-ups\n";
    std::cout << "  - Mushrooms make you big, flowers give you fireballs\n";
    std::cout << "  - Reach the flag pole at the end of each level to win\n";
    std::cout << "  - Collect 100 coins for an extra life\n";
    std::cout << "  - Hold run + stomp a shell to pick it up, release to throw\n";
    std::cout << "  - When you beat a level, all enemies turn into coins!\n";
    std::cout << "\n";
    std::cout << "TEST MODE (--test):\n";
    std::cout << "  - Mario is invincible (toggle with ` backtick key)\n";
    std::cout << "  - Time doesn't run out\n";
    std::cout << "  - Debug messages enabled\n";
    std::cout << "  - Special test map with access to all level types\n";
    std::cout << "  - Key I: Make Mario small\n";
    std::cout << "  - Key O: Make Mario big\n";
    std::cout << "  - Key P: Give Mario fire power\n";
    std::cout << "  - Keys 0-9: Spawn enemies at Mario's position\n";
    std::cout << "    0=Goomba, 1=Green Koopa, 2=Red Koopa, 3=Spiky\n";
    std::cout << "    4=Winged Goomba, 5=Winged Green Koopa\n";
    std::cout << "    6=Winged Red Koopa, 7=Winged Spiky\n";
    std::cout << "    8=Shell, 9=Bullet Bill\n";
    std::cout << "\n";
}

int main(int argc, char* argv[]) {
    bool useDefaultBindings = false;
    
    // Parse command line arguments first to set debug mode early
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printHelp(argv[0]);
            return 0;
        }
        if (strcmp(argv[i], "--debug") == 0 || strcmp(argv[i], "-d") == 0) {
            g_debugMode = true;
        }
        if (strcmp(argv[i], "--test") == 0 || strcmp(argv[i], "-t") == 0) {
            g_testMode = true;
            g_testInvincible = true;
            g_debugMode = true;  // Test mode implies debug mode
        }
        if (strcmp(argv[i], "--default") == 0) {
            useDefaultBindings = true;
        }
    }
    
    // Now output debug info if enabled
    if (g_debugMode) {
        setvbuf(stdout, NULL, _IONBF, 0);
        setvbuf(stderr, NULL, _IONBF, 0);
    }
    
    DEBUG_PRINT("main() entered");
    
    // Tell SDL we're handling main ourselves (must be called before SDL_Init)
    DEBUG_PRINT("Calling SDL_SetMainReady()");
    SDL_SetMainReady();
    DEBUG_PRINT("SDL_SetMainReady() done");
    
    if (g_testMode) {
        std::cout << "[TEST] Test mode enabled" << std::endl;
        std::cout << "[TEST] Mario is invincible (toggle with ` key)" << std::endl;
        std::cout << "[TEST] Press I=small, O=big, P=fire Mario" << std::endl;
        std::cout << "[TEST] Press 0-9 to spawn enemies" << std::endl;
    }
    
    if (useDefaultBindings) {
        std::cout << "[INPUT] Using default bindings (config file ignored)" << std::endl;
    }
    
    DEBUG_PRINT("Creating Game object...");
    Game game;
    
    DEBUG_PRINT("Calling game.init()...");
    if (!game.init(useDefaultBindings)) {
        std::cerr << "Failed to initialize game!" << std::endl;
        return 1;
    }
    
    DEBUG_PRINT("Calling game.run()...");
    game.run();
    
    DEBUG_PRINT("Calling game.cleanup()...");
    game.cleanup();
    
    DEBUG_PRINT("Exiting normally");
    return 0;
}

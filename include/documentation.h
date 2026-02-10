/**
 * @file documentation.h
 * @brief Master documentation file for Infinite Tux C++ Port
 * 
 * This file provides comprehensive Doxygen documentation for the entire
 * Infinite Tux codebase without modifying individual source files.
 * 
 * @mainpage Infinite Tux C++/SDL2 Port
 * 
 * @section intro Introduction
 * 
 * Infinite Tux is a C++/SDL2 port of the Java-based Infinite Mario Bros clone.
 * The game features procedurally generated levels with classic 2D platformer gameplay.
 * 
 * @section architecture Architecture Overview
 * 
 * The codebase is organized into several major components:
 * 
 * @subsection core Core Systems
 * - **Game**: Main game loop, SDL initialization, scene management
 * - **Scene**: Abstract base for all game screens (title, map, level, etc.)
 * - **Art**: Resource loading and management (sprites, sounds, music)
 * - **InputConfig**: Keyboard and gamepad input mapping
 * 
 * @subsection gameplay Gameplay Components
 * - **LevelScene**: Main gameplay scene with sprites, physics, collision
 * - **Level**: Tile map data and tile behavior lookup
 * - **LevelGenerator**: Procedural level generation from seed
 * - **LevelRenderer**: Tile rendering with camera scrolling
 * - **BgRenderer**: Parallax background rendering
 * 
 * @subsection sprites Sprite Hierarchy
 * - **Sprite**: Base class for all game objects
 *   - **Mario**: Player character with power-up states
 *   - **Enemy**: Base enemy with walking/hopping behavior
 *     - Goomba, Koopa (red/green), Spiky variants
 *   - **FlowerEnemy**: Piranha plant in pipes
 *   - **BulletBill**: Flying bullet enemy
 *   - **Shell**: Kicked/carried koopa shell
 *   - **Fireball**: Mario's fire projectile
 *   - **Mushroom**: Power-up mushroom item
 *   - **FireFlower**: Fire power-up item
 *   - **Particle**: Visual debris effects
 *   - **Sparkle**: Animated sparkle effects
 *   - **CoinAnim**: Coin pop-up animation
 * 
 * @subsection scenes Scene Types
 * - **TitleScene**: Main menu with options
 * - **MapScene**: World map navigation
 * - **LevelScene**: Main platforming gameplay
 * - **OptionsScene**: Control configuration
 * - **WinScene**: Victory screen
 * - **LoseScene**: Game over screen
 * 
 * @section gameloop Game Loop
 * 
 * The game uses a fixed-timestep loop (24 ticks/second) with interpolated rendering:
 * 
 * @code
 * while (running) {
 *     handleEvents();          // SDL events, input
 *     while (ticksToProcess > 0) {
 *         scene->tick();       // Fixed-step game logic
 *         ticksToProcess--;
 *     }
 *     scene->render(alpha);    // Interpolated rendering
 * }
 * @endcode
 * 
 * @section coordinates Coordinate System
 * 
 * - Origin: Top-left corner of the screen/level
 * - X: Increases rightward (in pixels)
 * - Y: Increases downward (in pixels)
 * - Tiles: 16x16 pixels each
 * - Screen: 320x240 native resolution, scaled to window
 * 
 * @section sprites_detail Sprite System Details
 * 
 * All game objects inherit from Sprite, which provides:
 * - Position (x, y) and velocity (xa, ya)
 * - Previous position (xOld, yOld) for interpolation
 * - Sprite sheet reference and frame selection
 * - Virtual methods for update and rendering
 * 
 * The tick/render cycle:
 * 1. tick() saves position and calls move()
 * 2. move() updates physics and game logic
 * 3. render() draws with interpolation for smooth animation
 * 
 * @section collision Collision Detection
 * 
 * Collisions are handled through virtual methods:
 * - collideCheck(): Called each tick to check Mario collision
 * - shellCollideCheck(): Check collision with moving shell
 * - fireballCollideCheck(): Check collision with fireball
 * - bumpCheck(): React to block being bumped from below
 * 
 * @section levelgen Level Generation
 * 
 * Levels are procedurally generated from a seed value using:
 * - Perlin noise for terrain variation
 * - Feature generators for platforms, pipes, gaps
 * - Enemy placement based on difficulty
 * - Guaranteed playability through constraint checking
 * 
 * The same seed always produces the same level, enabling
 * reproducible gameplay and infinite variety.
 * 
 * @section powerups Power-Up System
 * 
 * Mario has three states:
 * - **Small**: Default, one hit = death
 * - **Large**: Can break bricks, one hit = small
 * - **Fire**: Can shoot fireballs, one hit = large
 * 
 * Power-ups spawn from ? blocks:
 * - Mushroom appears if Mario is small
 * - Fire Flower appears if Mario is large
 * 
 * @section input Input System
 * 
 * InputConfig manages action bindings:
 * - Keyboard keys mapped to game actions
 * - Gamepad buttons, axes, and D-pad support
 * - Configuration saved to user directory
 * 
 * Game actions:
 * - MOVE_LEFT, MOVE_RIGHT, MOVE_UP, MOVE_DOWN
 * - JUMP, FIRE (also run/speed)
 * - PAUSE, MENU_SELECT, MENU_BACK
 * 
 * @section audio Audio System
 * 
 * Art class manages audio through SDL_mixer:
 * - Sound effects: WAV files, multiple can play simultaneously
 * - Music: MIDI files with synth options (Default, Native, FluidSynth)
 * 
 * @section building Building the Project
 * 
 * Requirements:
 * - CMake 3.16+
 * - C++17 compiler
 * - SDL2, SDL2_image, SDL2_mixer
 * 
 * Build commands:
 * @code
 * mkdir build && cd build
 * cmake ..
 * make
 * @endcode
 * 
 * @section files File Structure
 * 
 * @verbatim
 * infinite-tux-cpp/
 * ├── include/        Header files (.h)
 * ├── src/            Source files (.cpp)
 * ├── resources/      Game assets
 * │   ├── *.png       Sprite sheets
 * │   ├── snd/        Sound effects
 * │   └── mus/        Music files
 * ├── CMakeLists.txt  Build configuration
 * └── Doxyfile        Documentation config
 * @endverbatim
 * 
 * @author Infinite Tux C++ Port
 * @version 1.0.0
 */

/**
 * @defgroup core Core Systems
 * @brief Fundamental game systems
 * 
 * Core systems that form the backbone of the game:
 * - Game loop and SDL management
 * - Scene transitions
 * - Resource loading
 * - Input handling
 */

/**
 * @defgroup sprites Sprite System
 * @brief All game objects and entities
 * 
 * Sprites are the fundamental unit for all movable game objects:
 * - Player character (Mario)
 * - Enemies (Goomba, Koopa, etc.)
 * - Items (Mushroom, Fire Flower)
 * - Effects (Particles, Sparkles)
 */

/**
 * @defgroup scenes Game Scenes
 * @brief Different game screens/states
 * 
 * Each scene represents a distinct game state:
 * - Title screen
 * - World map
 * - Level gameplay
 * - Options menu
 * - Win/Lose screens
 */

/**
 * @defgroup level Level System
 * @brief Level data and generation
 * 
 * Level management including:
 * - Tile data storage
 * - Procedural generation
 * - Rendering with scrolling
 * - Background parallax
 */

/**
 * @defgroup audio Audio System
 * @brief Sound and music playback
 * 
 * Audio handled through SDL_mixer:
 * - Sound effects (WAV)
 * - Music tracks (MIDI)
 * - Synth selection
 */

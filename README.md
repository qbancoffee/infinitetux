# Infinite Tux v1.0.1 — Native SDL2 Edition

A C++ port of the original Java-based Infinite Tux, inspired by Notch's Infinite Mario Bros.
Built on SDL2 with full gamepad support.

## Features

- **C++ / SDL2** — ported from the original Java version
- **Gamepad support** — plug and play with configurable bindings
- Cross-platform (Linux, macOS, Windows)
- Procedurally generated levels with infinite replayability
- Classic platformer gameplay
- Sound effects and MIDI music with multiple synth options
- Multiple level types (overworld, underground, castle)
- Configurable keyboard controls
- Fullscreen and scaling options
- User-customizable sprites, sounds, and music

## Quick Start

### Pre-built Releases

Download pre-built binaries from the releases page:
- `infinite-tux-1.0.0-linux-x86_64.tar.gz` - Linux 64-bit
- `infinite-tux-1.0.0-windows-x86_64.zip` - Windows 64-bit

### Building from Source

See [Building](#building) section below.

## Requirements

- CMake 3.16+
- C++17 compiler (GCC 8+, Clang 7+, MSVC 2019+)
- SDL2, SDL2_image, SDL2_mixer

### Optional (for better MIDI music)
- FluidSynth
- FluidR3_GM soundfont (`fluid-soundfont-gm` package on Debian/Ubuntu)

---

## Building

### Quick Build (Platform-Specific Scripts)

Each platform has its own build script that handles everything:

```bash
# Linux x86_64
./build-linux-x86_64.sh

# Linux ARM64 (Raspberry Pi, etc.)
./build-linux-aarch64.sh

# Windows (cross-compile from Linux)
./build-windows-x86_64.sh

# macOS Intel
./build-macos-x86_64.sh

# macOS Apple Silicon
./build-macos-arm64.sh

# macOS Universal (Intel + Apple Silicon)
./build-macos-universal.sh
```

Build output goes to `build/` directory. Each script also creates a release package.

---

### Manual Build Instructions

### Linux (Ubuntu/Debian)

```bash
# Install dependencies
sudo apt-get install build-essential cmake libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev

# Optional: Better MIDI music
sudo apt-get install fluid-soundfont-gm

# Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Run
./InfiniteTux
```

### macOS

```bash
# Install dependencies
brew install cmake sdl2 sdl2_image sdl2_mixer

# Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(sysctl -n hw.ncpu)

# Run
./InfiniteTux
```

### Windows (MSVC + vcpkg)

1. Install [Visual Studio](https://visualstudio.microsoft.com/) with C++ support
2. Install [vcpkg](https://github.com/microsoft/vcpkg):
```powershell
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
```

3. Install SDL2:
```powershell
.\vcpkg install sdl2:x64-windows sdl2-image:x64-windows sdl2-mixer:x64-windows
```

4. Build:
```powershell
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]\scripts\buildsystems\vcpkg.cmake
cmake --build . --config Release
```

### Windows (Cross-compile from Linux)

Use the provided script which downloads SDL2 automatically:

```bash
# Install MinGW
sudo apt-get install mingw-w64 cmake curl zip

# Build
./build-windows-x86_64.sh
```

The script downloads SDL2 libraries and packages everything with required DLLs.

---

## Controls

### Keyboard
| Key | Action |
|-----|--------|
| Arrow Keys | Move left/right, climb vines, duck (down) |
| A / Z | Jump (hold longer to jump higher) |
| S / X | Run, shoot fireballs, pick up shells |
| Enter | Pause/unpause game |
| Escape | Quit game |

### Gamepad

Gamepads are automatically detected. Configure bindings via the Options menu.

| Button | Action |
|--------|--------|
| D-pad / Left Stick | Move |
| A | Jump |
| B / X | Run, Fire |
| Start | Pause |

### Display Options
| Key | Action |
|-----|--------|
| F5 | Decrease sound effects volume |
| F6 | Increase sound effects volume |
| F7 | Decrease music volume |
| F8 | Increase music volume |
| F9 | Cycle MIDI synth (Default/Native/FluidSynth) |
| F10 | Cycle scale quality (Nearest/Linear/Best) |
| F11 | Toggle fullscreen |

### Test Mode (--test flag)
| Key | Action |
|-----|--------|
| ` (backtick) | Toggle invincibility |
| I | Make Mario small |
| O | Make Mario big |
| P | Give Mario fire power |
| 0-9 | Spawn enemies |

## Soundfont Configuration

The game supports custom SoundFont files (.sf2) for MIDI music playback. You can configure different soundfonts for each music track.

### Setting Up Soundfonts

1. Place your .sf2 files in the `resources/soundfonts/` directory
2. Configure via Options menu (Audio Settings) or edit the config file directly

### Config File

The config file is located at:
- Linux: `~/.config/infinite-tux-input.cfg`
- Windows: `%APPDATA%\infinite-tux-input.cfg`

Soundfont section example:
```ini
[soundfonts]
default = FluidR3_GM.sf2
title = 
overworld = GeneralUser_GS.sf2
underground = 
castle = Arachno.sf2
map = 
```

- `default`: Soundfont used for all tracks unless overridden
- Per-track settings override the default
- Empty value = use system default soundfont

### Recommended Soundfonts

- **FluidR3_GM** - High quality, good all-around (~150MB)
- **GeneralUser GS** - Excellent quality, smaller size (~30MB)
- **Arachno** - Good for retro sounds (~150MB)

## Command Line Options

```
./InfiniteTux [OPTIONS]

  -h, --help      Show help message
  -d, --debug     Enable debug output
  -t, --test      Enable test mode (invincibility, debug keys)
  --default       Use default input bindings (reset config)
```

## Gameplay Tips

- Stomp enemies by landing on them from above
- Hit ? blocks from below to get coins and power-ups
- Mushrooms make you big, flowers give you fireballs
- Reach the flag pole at the end of each level to win
- Collect 100 coins for an extra life
- Hold run + stomp a shell to pick it up, release to throw
- When you beat a level, all enemies turn into coins!

---

## Custom Resources

You can customize the game by replacing sprites, sounds, and music with your own files.

### User Data Directory

Place custom files in the user data directory:
- **Linux**: `~/.local/share/infinitetux/`
- **Windows**: `%APPDATA%\infinitetux\`

The game creates this directory automatically on first run, with a README.txt
listing all overridable files.

### Directory Structure

```
~/.local/share/infinitetux/
├── README.txt          - Auto-generated documentation
├── snd/                - Custom sound effects (WAV)
├── mus/                - Custom music files (MIDI)
├── soundfonts/         - Custom soundfonts (SF2)
└── (image files)       - Sprite sheets and images
```

### How It Works

Files in the user directory take priority over system resources. You only need
to provide files for the resources you want to change.

For detailed information, see [RESOURCE-OVERRIDES.md](RESOURCE-OVERRIDES.md).

---

## Project Structure

```
infinite-tux-cpp/
├── CMakeLists.txt            # Build configuration
├── README.md                 # This file
├── VERSION                   # Version number
├── build-linux-x86_64.sh     # Linux x86_64 build script
├── build-linux-aarch64.sh    # Linux ARM64 build script
├── build-windows-x86_64.sh   # Windows cross-compile script
├── build-macos-x86_64.sh     # macOS Intel build script
├── build-macos-arm64.sh      # macOS Apple Silicon build script
├── build-macos-universal.sh  # macOS Universal build script
├── include/                  # Header files
├── src/                      # Source files
└── resources/                # Game assets
    ├── *.png                 # Sprite sheets
    ├── snd/                  # Sound effects (.wav)
    ├── mus/                  # Music files (.mid)
    └── soundfonts/           # Custom soundfonts (.sf2)
```

## Version History

### v1.0.0 (2024)
- Initial C++/SDL2 port from Java
- Complete gameplay implementation
- Procedural level generation
- All enemy types (Goomba, Koopa, Spiky, Piranha Plant, Bullet Bill)
- Shell carrying and throwing mechanics
- Fire Mario with fireballs
- Multiple level types (overworld, underground, castle)
- World map navigation
- Win/lose scenes
- Configurable controls with gamepad support
- MIDI music with multiple synth options (Default, Native, FluidSynth)
- Fullscreen mode with scale quality options
- Test mode for debugging
- Enemies convert to coins when level is completed
- Cross-compilation support for Windows and macOS

## License

Original game by Markus Persson (Notch).
Tux sprites and modifications for Infinite Tux are open source.
See the original project's license for details.

## Credits

- Original Java game: Markus "Notch" Persson (Infinite Mario Bros.)
- Infinite Tux modifications: See original project
- C++/SDL2 port: This project

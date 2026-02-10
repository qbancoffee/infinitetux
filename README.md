# Infinite Tux v1.0.1  Native SDL2 Edition

A C++ port of the original Java-based Infinite Tux, inspired by Notch's Infinite Mario Bros.
Built on SDL2 with full gamepad support.

This is a debianized source package that can be built using standard Debian packaging tools.

## Features

- **C++ / SDL2** — ported from the original Java version
- **Gamepad support** — plug and play with configurable bindings
- Procedurally generated levels with infinite replayability
- Classic platformer gameplay
- Sound effects and MIDI music with multiple synth options
- Multiple level types (overworld, underground, castle)
- Configurable keyboard controls
- Fullscreen and scaling options
- User-customizable sprites, sounds, and music

## Building

### Debian Package (Recommended)

Build an installable `.deb` package using standard Debian tools:

```bash
# Install build dependencies
sudo apt-get install build-essential debhelper cmake \
    libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev

# Build the package
dpkg-buildpackage -us -uc

# Install the resulting package
sudo dpkg -i ../infinitetux_*.deb

# Install runtime dependency for MIDI music
sudo apt-get install fluid-soundfont-gm
```

The game will be installed to `/usr/games/infinitetux` with resources in
`/usr/share/games/infinitetux/`.

### Manual Build (CMake)

For development or non-Debian systems:

```bash
# Install dependencies (Debian/Ubuntu)
sudo apt-get install build-essential cmake libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev

# Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Run from build directory
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

### Windows (Cross-compile from Linux)

```bash
# Install MinGW
sudo apt-get install mingw-w64 cmake curl zip

# Build (downloads SDL2 automatically)
./build-windows-x86_64.sh
```

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

## Command Line Options

```
infinitetux [OPTIONS]

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

Files in the user directory take priority over system resources.

For detailed information, see [RESOURCE-OVERRIDES.md](RESOURCE-OVERRIDES.md).

---

## Soundfont Configuration

The game supports custom SoundFont files (.sf2) for MIDI music playback.

### Config File

Located at `~/.config/infinite-tux-input.cfg`:

```ini
[soundfonts]
default = FluidR3_GM.sf2
title = 
overworld = GeneralUser_GS.sf2
underground = 
castle = Arachno.sf2
map = 
```

### Recommended Soundfonts

- **FluidR3_GM** - High quality, good all-around (~150MB)
- **GeneralUser GS** - Excellent quality, smaller size (~30MB)
- **Arachno** - Good for retro sounds (~150MB)

---

## Project Structure

```
infinite-tux/
├── CMakeLists.txt        # Build configuration
├── debian/               # Debian packaging files
│   ├── control           # Package metadata
│   ├── rules             # Build rules
│   └── ...
├── include/              # Header files
├── src/                  # Source files
└── resources/            # Game assets
    ├── *.png             # Sprite sheets
    ├── snd/              # Sound effects (.wav)
    ├── mus/              # Music files (.mid)
    └── soundfonts/       # Custom soundfonts (.sf2)
```

## License

This project is licensed under GPL-3+.

Original game logic by Markus Persson (public domain).
C++ port and Tux sprites by Pedro Pena and contributors.

See `debian/copyright` for detailed licensing information for all assets.

## Credits

- Original Java game: Markus "Notch" Persson (Infinite Mario Bros.)
- Infinite Tux (Java): Pedro Pena
- C++/SDL2 port: Pedro Pena and contributors

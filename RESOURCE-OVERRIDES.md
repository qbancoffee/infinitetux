# Infinite Tux - Custom Resource Overrides

Infinite Tux supports overriding any game resource (sprites, sounds, music) with
custom files. This allows you to create custom themes, sprite replacements, or
sound packs without modifying the system installation.

## User Data Directory

Custom resources are placed in the user data directory:

- **Linux**: `~/.local/share/infinitetux/`
- **Windows**: `%APPDATA%\infinitetux\`

This directory follows the XDG Base Directory Specification on Linux. If the
`XDG_DATA_HOME` environment variable is set, it will be used instead of the
default `~/.local/share/`.

## How It Works

When the game loads a resource, it checks the user data directory first. If a
file with the matching name exists there, it is loaded instead of the default
system resource. This allows selective overriding - you only need to provide
files for the resources you want to change.

## Directory Structure

The user data directory has the following structure:

```
~/.local/share/infinitetux/
├── README.txt        - Auto-generated documentation
├── snd/              - Custom sound effects (WAV)
├── mus/              - Custom music files (MIDI)
├── soundfonts/       - Custom soundfonts (SF2)
└── (image files)     - Sprite sheets and images (PNG/GIF)
```

## Available Resources

### Sprite Sheets (place in main directory)

| File | Description | Tile Size |
|------|-------------|-----------|
| `mariosheet.png` | Big Mario sprites | 32x32 |
| `smallmariosheet.png` | Small Mario sprites | 16x16 |
| `firemariosheet.png` | Fire Mario sprites | 32x32 |
| `enemysheet.png` | Enemy sprites | 16x32 |
| `itemsheet.png` | Items (coins, mushrooms, flowers) | 16x16 |
| `mapsheet.png` | Level tiles | 16x16 |
| `worldmap.png` | World map tiles | 16x16 |
| `particlesheet.png` | Particle effects | 8x8 |
| `bgsheet.png` | Background tiles | 32x32 |
| `font.gif` | Font characters | 8x8 |
| `endscene.gif` | End scene graphic | 96x96 |
| `gameovergost.gif` | Game over ghost | 96x64 |
| `logo.gif` | Game logo | - |
| `title.gif` | Title screen background | - |

### Data Files (place in main directory)

| File | Description |
|------|-------------|
| `tiles.dat` | Tile behavior data (256 bytes) |

### Sound Effects (place in `snd/` subdirectory)

| File | Description |
|------|-------------|
| `breakblock.wav` | Block breaking |
| `coin.wav` | Coin collection |
| `jump.wav` | Mario jump |
| `stomp.wav` | Enemy stomp |
| `kick.wav` | Shell kick |
| `powerup.wav` | Power-up collection |
| `powerdown.wav` | Taking damage |
| `death.wav` | Mario death |
| `sprout.wav` | Item appearing from block |
| `cannon.wav` | Bullet Bill cannon |
| `bump.wav` | Bump/collision sound |
| `exit.wav` | Level exit |
| `1-up.wav` | Extra life |
| `fireball.wav` | Fireball throw |
| `lowtime.wav` | Low time warning |

### Music Files (place in `mus/` subdirectory)

| File | Description |
|------|-------------|
| `smb3map1.mid` | World map music |
| `smwovr1.mid` | Overworld level music |
| `smb3undr.mid` | Underground level music |
| `smwfortress.mid` | Castle/fortress music |
| `smwtitle.mid` | Title screen music |

### Soundfonts (place in `soundfonts/` subdirectory)

Place `.sf2` SoundFont files here for custom MIDI instrument sounds.

## Creating Custom Resources

### Sprite Sheets

When creating custom sprite sheets:

1. Maintain the same dimensions and frame layout as the originals
2. Keep the same number of frames in each row/column
3. Use PNG format with transparency for sprites
4. GIF format is also supported for some resources

### Sound Effects

- Use WAV format (PCM recommended)
- Match the approximate duration of original sounds
- 44100 Hz sample rate recommended

### Music

- Use MIDI format (.mid)
- Standard General MIDI instruments work best
- Custom soundfonts can be used for different instrument sounds

## Debugging

Run the game with the `--debug` or `-d` flag to see which resource files are
being loaded:

```bash
infinitetux --debug
```

This will show messages like:
```
[DEBUG] Using user override: /home/user/.local/share/infinitetux/mariosheet.png
```

## First Run

On first launch, the game automatically creates the user data directory
structure and a README.txt file with detailed information about each
overridable resource.

## Examples

### Replace Mario's Sprites

1. Create a `mariosheet.png` with your custom big Mario sprites (32x32 per frame)
2. Place it in `~/.local/share/infinitetux/mariosheet.png`
3. Restart the game

### Replace Jump Sound

1. Create a custom `jump.wav` sound effect
2. Place it in `~/.local/share/infinitetux/snd/jump.wav`
3. Restart the game

### Use Custom Music

1. Create or obtain MIDI files for game music
2. Name them according to the table above (e.g., `smwovr1.mid`)
3. Place them in `~/.local/share/infinitetux/mus/`
4. Restart the game

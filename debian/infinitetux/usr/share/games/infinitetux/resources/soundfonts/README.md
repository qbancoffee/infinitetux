# Soundfonts Directory

Place SoundFont files (.sf2) in this directory to use them for MIDI music playback.

## Configuration

Soundfonts can be configured in two ways:

### 1. Via Options Menu (F2 in game)
Navigate to the Audio section and select soundfonts for each music track.

### 2. Via Config File
Edit the `[soundfonts]` section in your config file:
- Linux: `~/.config/infinite-tux-input.cfg`
- Windows: `%APPDATA%\infinite-tux-input.cfg`

Example configuration:
```ini
[soundfonts]
default = FluidR3_GM.sf2
title = FluidR3_GM.sf2
overworld = GeneralUser_GS.sf2
underground = FluidR3_GM.sf2
castle = Arachno.sf2
map = FluidR3_GM.sf2
```

## Soundfont Sources

Free soundfonts can be downloaded from:
- FluidR3_GM: https://member.keymusician.com/Member/FluidR3_GM/
- GeneralUser GS: https://schristiancollins.com/generaluser.php
- Arachno: https://www.arachnosoft.com/main/soundfont.php

## System Soundfonts

If no soundfont is specified or found, the game will try these system locations:
- Linux: `/usr/share/sounds/sf2/FluidR3_GM.sf2`
- Windows: `C:\soundfonts\default.sf2`

## Notes

- Only .sf2 (SoundFont 2) files are supported
- Larger soundfonts provide better quality but use more memory
- FluidSynth must be available in SDL_mixer for soundfonts to work
- Press F9 in-game to cycle between Default/Native/FluidSynth MIDI modes

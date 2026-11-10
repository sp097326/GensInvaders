# WARP.md

This file provides guidance to WARP (warp.dev) when working with code in this repository.

## Project Overview

GensInvaders is a Space Invaders-style game for the Sega Genesis 16-bit console, developed using SGDK (Sega Genesis Development Kit). It's based on the tutorial from ohsat.com but includes custom modifications and improvements.

## Development Environment

### Prerequisites
- SGDK (Sega Genesis Development Kit) installed at `C:\tools\SGDK`
- Environment variable `GDK` set to the SGDK installation path
- Visual Studio Code with Genesis-Code extension (optional but recommended)

### Core Technologies
- **Language**: C (68000 assembly for low-level boot code)
- **Framework**: SGDK (Sega Genesis Development Kit)
- **Target Platform**: Sega Genesis/Mega Drive
- **Resource Management**: SGDK resource compiler for sprites, sounds, and graphics

## Build Commands

Since this is an SGDK project, compilation is handled by the SGDK toolchain:

```bash
# Build the ROM (typically done through SGDK's make system)
# From the project root directory:
make clean
make

# Alternative build command if using SGDK directly:
$env:GDK\bin\make clean
$env:GDK\bin\make
```

The build process generates:
- `out/rom.bin` - The final Genesis ROM file
- `out/rom.out` - Debug version with symbols for GDB debugging
- `out/symbol.txt` - Symbol table for debugging

## Development Workflow

### Running and Testing
```bash
# The generated ROM can be run in Genesis emulators like:
# - Gens KMod (supports debugging)
# - BlastEm
# - RetroArch with Genesis Plus GX core

# For debugging with GensKMod:
# 1. Load the ROM in GensKMod
# 2. Enable debug mode (port 6868)
# 3. Use VS Code debug configuration "Debug with gdb remote"
```

### Asset Management
Assets are managed through the SGDK resource system:
- `res/resources.res` - Resource definition file
- `res/resources.h` - Generated header with asset definitions
- Assets include sprites (`tiles/`), sounds (`sounds/`), and music (`music/`)

## Code Architecture

### High-Level Structure

The game follows a traditional game loop architecture optimized for the Genesis hardware:

```
main.c
├── Game Entity System
│   ├── GameEntity struct (position, velocity, health, sprite)
│   ├── Player management
│   ├── Enemy array management (MAX_ENEMIES = 7)
│   └── Bullet pool system (MAX_BULLETS = 6)
├── Core Game Loop
│   ├── Input handling (JOY_1 controller)
│   ├── Physics/Movement updates
│   ├── Collision detection
│   └── Rendering (SGDK Sprite Engine)
├── Audio System
│   ├── PCM sound effects (laser, explosion)
│   └── Music playback (intro music)
└── Powerup System
    └── RapidFire powerup implementation
```

### Key Architectural Patterns

**Entity-Component Pattern**: All game objects (player, enemies, bullets) use the `GameEntity` struct with position, dimensions, velocity, health, and sprite references.

**Object Pooling**: Bullets use a pre-allocated pool system to avoid dynamic memory allocation during gameplay, critical for Genesis performance.

**State Management**: Game state is managed through global variables and entity health values, with visibility controlled through SGDK sprite system.

**Hardware-Specific Optimizations**:
- Uses SGDK's VSync waiting (`SYS_doVBlankProcess()`)
- Implements proper interrupt handling (`SYS_disableInts()/enableInts()`)
- Uses DMA transfers for palette and tileset loading
- Scrolling background implementation with vertical offset

### Resource Management

Assets are compiled into the ROM through SGDK's resource compiler:
- **Sprites**: Defined with frame count, collision boxes, and animation data
- **Audio**: WAV files converted to PCM format with compression levels
- **Graphics**: BMP files converted to Genesis tile format with palette optimization

## File Structure Significance

```
src/
├── main.c              # Main game logic and loop
└── boot/
    ├── rom_head.c      # Genesis ROM header definition
    └── sega.s          # Genesis boot assembly code

res/
├── resources.res       # Asset definition file
├── resources.h         # Generated asset headers
├── tiles/             # Sprite and background graphics
├── sounds/            # Sound effects (WAV format)
└── music/             # Background music (WAV format)

out/                   # Build output directory
├── rom.bin           # Final Genesis ROM
├── rom.out           # Debug ROM with symbols
└── symbol.txt        # Debug symbol table
```

## Development Considerations

### Genesis Hardware Constraints
- **Memory**: Limited to 64KB RAM, careful memory management required
- **CPU**: 68000 at 7.67MHz, optimize for performance
- **Graphics**: 64 colors on screen, 4 palettes of 16 colors each
- **Audio**: 6-channel FM + 4-channel PSG + 1 PCM channel

### Performance Patterns
- Use sprite pooling instead of dynamic allocation
- Minimize VDP operations outside VBlank
- Batch palette and tileset updates using DMA
- Keep game logic frame-rate independent

### Common Issues
- Sound effects may not work (known issue in codebase)
- Performance can degrade with complex background generation
- Sprite animation timing depends on frame rate

## Debugging Setup

The project includes VS Code debugging configuration:
- Debug target: `out/rom.out`
- Debug server: GensKMod on localhost:6868
- GDB path: `${env:GDK}\bin\gdb.exe`
- Source mapping for SGDK library files

Use GensKMod emulator for debugging Genesis ROMs with breakpoint and memory inspection support.

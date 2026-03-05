# PacMan — iGraphics (OpenGL/GLUT)

A Pac-Man inspired 2D arcade game built in **C++** using the **iGraphics** framework (a thin wrapper around OpenGL/GLUT), designed to run on **Windows** and compiled with **Code::Blocks + MinGW**.

---

## Table of Contents

- [Demo / Screenshots](#demo--screenshots)
- [Features](#features)
- [Project Structure](#project-structure)
- [Prerequisites](#prerequisites)
- [Setup in Code::Blocks](#setup-in-codeblocks)
- [Controls](#controls)
- [Gameplay](#gameplay)
- [Configuration](#configuration)
- [Known Issues](#known-issues)
- [License](#license)

---

## Features

- Classic Pac-Man movement across a multi-ring maze
- 4-directional movement with wall collision detection for every maze boundary
- Teleportation tunnels (left/right and top/bottom)
- Food pellets placed across 3 concentric maze rings — eaten on contact
- Lives system (3 lives displayed as HUD icons)
- Score counter updated in real time
- Elapsed time display (MM:SS) on HUD
- Background music (WAV) via Windows `PlaySound` API
- Intro screen → Main menu → Gameplay flow
- BMP sprite assets for Pac-Man (4 directional frames), food, and life icons

---

## Project Structure

```
MyiGraphics/
├── iMain.cpp           # Main game source file
├── iGraphics.h         # iGraphics framework header (OpenGL/GLUT wrapper)
├── MyiGraphics.cbp     # Code::Blocks project file
├── intro.wav           # Background music (place in project root)
├── highscore.txt       # High score persistence file
├── pic1/               # All BMP sprite assets
│   ├── intro.bmp       # Intro screen background
│   ├── option.bmp      # Main menu background
│   ├── play.bmp        # Gameplay background
│   ├── pacr.bmp        # Pac-Man facing right
│   ├── pacl.bmp        # Pac-Man facing left
│   ├── pacu.bmp        # Pac-Man facing up
│   ├── pacd.bmp        # Pac-Man facing down
│   ├── life.bmp        # Life icon
│   └── f0.bmp          # Food pellet sprite
├── bin/                # Compiled output (auto-generated)
└── obj/                # Object files (auto-generated)
```

---

## Prerequisites

Before setting up, ensure you have the following installed:

| Requirement | Details |
|---|---|
| **Code::Blocks** | Version 17.12 or later — [codeblocks.org](http://www.codeblocks.org/downloads) |
| **MinGW (GCC)** | Bundled with Code::Blocks (`codeblocks-XX.XXmingw-setup.exe`) |
| **GLUT 3.2 (glut32)** | Legacy GLUT for Windows |
| **glaux** | OpenGL auxiliary library |
| **Windows OS** | Required (uses `windows.h`, `mmsystem.h`, `PlaySound`) |

### Download GLUT / glaux files

You need the following files from the GLUT 3.2 package:

| File | Destination |
|---|---|
| `glut.h` | `C:\MinGW\include\GL\` |
| `glaux.h` | `C:\MinGW\include\GL\` |
| `glut32.lib` | `C:\MinGW\lib\` |
| `glaux.lib` | `C:\MinGW\lib\` |
| `glut32.dll` | `C:\Windows\System32\` (and `C:\Windows\SysWOW64\` on 64-bit) |

> Download link: [GLUT for Win32](https://www.transmissionzero.co.uk/software/freeglut-devel/) or search for `glut-3.7.6-bin.zip`.

---

## Setup in Code::Blocks

### Step 1 — Install Code::Blocks with MinGW

Download and install the **MinGW bundled** installer from [codeblocks.org](http://www.codeblocks.org/downloads/binaries).  
Choose: `codeblocks-XX.XXmingw-setup.exe`

---

### Step 2 — Install GLUT and glaux

1. Download the GLUT 3.2 package (`glut-3.7.6-bin.zip` or equivalent).
2. Copy the files to the locations listed in the table above.

---

### Step 3 — Open the Project

1. Launch **Code::Blocks**.
2. Go to **File → Open** and navigate to the project folder.
3. Select **`MyiGraphics.cbp`** and click **Open**.

---

### Step 4 — Configure the Compiler Include Path (if needed)

If Code::Blocks cannot find `gl/glut.h`:

1. Go to **Settings → Compiler**.
2. Select the **Search directories** tab → **Compiler** sub-tab.
3. Add the path: `C:\MinGW\include`
4. Switch to the **Linker** sub-tab.
5. Add the path: `C:\MinGW\lib`

---

### Step 5 — Verify Linker Libraries

The project already has these configured in `MyiGraphics.cbp`, but if you see linker errors:

1. Right-click the project in the **Project tree** → **Build options**.
2. Select the **Linker settings** tab.
3. Ensure the following libraries are listed under **Link libraries**:
   - `opengl32`
   - `glu32`
   - `glut32`
   - `glaux`

---

### Step 6 — Build and Run

1. Press **F9** (Build and Run), or go to **Build → Build and Run**.
2. The game window (`1900 × 950`) will open.

> **Tip:** If the window appears off-screen, reduce `SCREEN_WIDTH` / `SCREEN_HEIGHT` in `iMain.cpp` to match your display resolution.

---

## Controls

| Key | Action |
|---|---|
| `S` | Start — dismiss intro screen and open the main menu |
| `Arrow Up` | Move Pac-Man up |
| `Arrow Down` | Move Pac-Man down |
| `Arrow Left` | Move Pac-Man left |
| `Arrow Right` | Move Pac-Man right |
| **Left Mouse Button** | Click **PLAY** button on the main menu |

---

## Gameplay

1. **Intro Screen** — Press `S` to continue.
2. **Main Menu** — Click the **PLAY** button to start the game.
3. **Gameplay**:
   - Navigate Pac-Man through the maze using the arrow keys.
   - Eat all food pellets to score points (+10 per pellet).
   - Use the four tunnels (left/right sides and top/bottom) to teleport across the map.
   - Your **score** and elapsed **time** are shown in the HUD.
   - You have **3 lives** shown in the top-right area.

### Maze Layout

The maze consists of **4 concentric rings**:

| Ring | Description |
|---|---|
| M1 (outermost) | Large border with 4 tunnel openings |
| M2 | Second ring with gap passages |
| M3 | Third ring with inner passages |
| M4 (innermost) | Small inner box around the center block |

---

## Configuration

Key constants in `iMain.cpp` can be adjusted:

| Constant | Default | Description |
|---|---|---|
| `SCREEN_WIDTH` | `1900` | Window width in pixels |
| `SCREEN_HEIGHT` | `950` | Window height in pixels |
| `PAC_MOVE_DELAY` | `15` (ms) | Timer interval for movement & collision (lower = faster) |
| `FOOD_SPACING` | `40` | Pixel gap between food pellets |
| `musicEnabled` | `true` | Toggle background music on/off |

---

## Known Issues

- **Window size**: The default resolution (`1900×950`) may exceed smaller displays. Adjust `SCREEN_WIDTH` and `SCREEN_HEIGHT`.
- **Missing `intro.wav`**: If the WAV file is absent, the game will silently skip music. No crash occurs.
- **glaux deprecated**: `glaux` is a legacy library and may produce warnings on newer compilers — this is expected and does not affect functionality.
- **64-bit MinGW**: Some 64-bit MinGW builds do not ship `glut32`/`glaux`. Use the 32-bit MinGW toolchain bundled with Code::Blocks for best compatibility.

---

## License

This project is licensed under the terms found in the [LICENSE](LICENSE) file.

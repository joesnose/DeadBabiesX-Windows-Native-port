# DeadBabiesX — Windows Native Port

Native Windows port of Dead Babies Xtreme, the Xbox homebrew game by **Chris Friesen** and credited contributors. Built from the available C++ game source with native Windows graphics, audio, and input.

**[Download the latest Windows x64 build](downloads/DeadBabiesX-Windows-Alpine-Dusk-x64-v1.2.zip?raw=true)** · [SHA256 checksum](downloads/SHA256SUMS)

The latest Windows build adds Alpine Dusk graphics and makes Exit Game in the pause menu return to the main menu. The prior release remains available.

## Run

The current download uses a standard uncompressed ZIP for extraction compatibility.

Extract the full ZIP, open the `DeadBabiesX-Windows` folder, and run **`DeadBabiesXNative.exe`** directly. Keep the `game/` folder and `libwinpthread-1.dll` beside it.

The four `.ogg` files must also stay beside the EXE because the Music menu loads them from that location:

| File | Music option |
|---|---|
| `music3.ogg` | Original |
| `static-reactor.ogg` | Static Reactor |
| `acid-house.ogg` | Acid House |
| `rock.ogg` | Rock |

All optional `.cmd` launchers have been removed. Graphics and music can be selected from the game menus. The executable is rebuilt for this update; the required runtime DLL is bundled.

For Steam on Linux, run the Windows executable using a compatible Proton version.

## Features

- Original, HD, Super HD, Super Super HD, Cel Shaded, Cartoon, and Alpine Dusk graphics.
- Alpine Dusk: an original faceted mountain-and-pine background, a pastel dusk palette, cooler terrain, and simplified sprite shading inspired by over the hill. It retains the 2D game and original sprite silhouettes.
- Pause menu Exit Game returns to the main menu; Exit Game on the main menu still closes the application.
- Four selectable music tracks, including Rock.
- Local controller multiplayer and optional Mega Weapons.
- Title/pause menus and animated winner/score results.
- Louder music, game-over speech, spoken winner cue, and victory sting.

## Controls

Controller: left stick moves, right stick aims, RT fires, LT alternate fire, A collects pickups/confirms menus, RB/LB activate specials, and Start joins/pauses.

Keyboard: A/D move, arrow keys aim, Space fires, Shift alternate fire, Q/E specials, Enter confirms/joins/pauses, and Escape opens the pause menu. F11 or Alt+Enter switches fullscreen/windowed mode.

## Source and build

`source/` contains the native C++ source, including gameplay, frontend, renderer, audio mixer, original-source attribution, and art/music tooling. `assets/` contains the required game assets and four music tracks. The original Xbox binary recompilation toolkit is not part of this port.

With CMake 3.20+ and Visual Studio's C++ tools/Windows SDK:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
```

The executable is under `build/source/Release/`. The root CMake project copies `assets/` beside it. When building with MinGW-w64, also supply its `libwinpthread-1.dll` beside the executable, as in the download.

The ready-to-play ZIP contains runtime files and notices; source and development files are kept separately in this repository.

## Credits

Original game/code/art: Chris Friesen and the contributors named in [the original game readme](notices/ORIGINAL-GAME.txt). The native port and subsequent additions were developed with AI assistance. Original credits and component licences remain applicable; see `notices/` and the notices embedded in vendored source. No new blanket licence is applied to the game or artwork.

Android is maintained separately: [DeadBabiesX Android](https://github.com/joesnose/DeadBabiesX-Android-recompilation).

## Alpine Dusk update validation

Built with MinGW-w64 for Windows x64. Checks passed for 160 graphics-mode
transitions, missing-pack handling, pause versus title Exit actions, palette
alpha preservation, and package integrity. Windows/Proton gameplay, audio and
controller hardware testing were not available in the build environment.
The original 4:3 viewport, four music tracks and earlier graphics modes remain.

Reference: https://store.steampowered.com/app/2929250/ . The new background is
original generated artwork. No assets from over the hill are distributed.

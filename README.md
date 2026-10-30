# DeadBabiesX — Windows Native Port

Native Windows port of Dead Babies Xtreme, the Xbox homebrew game by **Chris Friesen** and credited contributors. Built from the available C++ game source with native Windows graphics, audio, and input.

**[Download the latest Windows x64 build](downloads/DeadBabiesX-Windows-Native-x64.zip?raw=true)** · [SHA256 checksum](downloads/SHA256SUMS)

This repository contains one Windows build: the final Rock-and-louder-announcements version. Earlier builds and unrelated projects are excluded.

## Run

Extract the full ZIP and run **`DeadBabiesXNative.exe`** directly. Keep the `game/` folder and `libwinpthread-1.dll` beside it.

The four `.ogg` files must also stay beside the EXE because the Music menu loads them from that location:

| File | Music option |
|---|---|
| `music3.ogg` | Original |
| `static-reactor.ogg` | Static Reactor |
| `acid-house.ogg` | Acid House |
| `rock.ogg` | Rock |

All optional `.cmd` launchers have been removed. Graphics and music can be selected from the game menus. The executable and runtime DLL are unchanged from the latest Windows package.

For Steam on Linux, run the Windows executable using a compatible Proton version.

## Features

- Original, HD, Super HD, Super Super HD, Cel Shaded, and Cartoon graphics.
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

# DeadBabiesX — Windows Native Port

Native Windows port of Dead Babies Xtreme, the Xbox homebrew game by **Chris Friesen** and credited contributors. Built from the available C++ game source with native Windows graphics, audio, and input.

**[Download the latest Windows x64 build](downloads/DeadBabiesX-Windows-v1.5.1-Train-Fix.zip?raw=true)** · [SHA256 checksum](downloads/SHA256SUMS)

The latest Windows build upgrades Super Super HD with a dedicated Cinematic art pack. Cartoon, Alpine Dusk and the pause-menu fix remain intact. The prior release remains available.

## Run

The current download uses standard ZIP/Deflate compression, supported by Windows Explorer and common ZIP tools.

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
- Alpine Dusk: an original faceted mountain-and-pine background, a pastel dusk palette, cooler terrain, and a complete redrawn sprite set inspired by over the hill. It retains the 2D game and original sprite silhouettes.
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

Reference: https://store.steampowered.com/app/2929250/ . The background and sprite redraws are
original generated artwork. No assets from over the hill are distributed.

## v1.3 Alpine sprite pack

Alpine Dusk now supplies dedicated artwork for all 109 non-background PNG
textures: 40 player parts/icons across four colours, 25 enemy/balloon frames,
14 pickups, 28 weapon/effect/cursor/debris textures, and two HUD textures.
Every sprite exports on a canvas four times the original dimensions. Alpha
masks and pivots come from the original textures; game collision and animation
code are unchanged. Life icons reuse matching torso/leg artwork. The v1.2
landscape is unchanged. Other graphics modes continue using their own artwork.

Atlas source sheets and layout are in source/alpine_art. With Pillow installed,
re-export using `python source/export_alpine_sprites.py --atlases source/alpine_art`.
Run `python source/test_alpine_assets.py` to check full coverage and alpha masks.

## v1.4 Classic Cartoon upgrade

Select the existing **Cartoon** graphics option for a complete hand-drawn-style
sprite pack: solid colour fills, bold dark contours, rounded forms and simple
cel shadows. All 109 non-background sprite PNGs are supplied: players in four
colours, enemies and balloon frames, pickups, weapons, effects, cursors, debris
and HUD. Canvas resolution is 4x the original; module positions and pivots
retain animation alignment. Life icons use matching torso and leg artwork.
Alpine Dusk artwork is unchanged. Backgrounds and any missing custom textures
retain the existing Cartoon filter fallback.

Atlas sources are in source/cartoon_art. Re-export with
`python source/export_alpine_sprites.py --theme cartoon --atlases source/cartoon_art`.
Validate with `python source/test_alpine_assets.py --theme cartoon`.
The shared exporter supports both complete art packs. Native Windows/Proton
gameplay and live rendering have not been tested in this environment.

## v1.5 Cinematic Super Super HD

The existing **Super Super HD** menu option selects a new dedicated art pack
with detailed adult player/equipment materials, polished stylized toy enemies,
refined VFX, a premium HUD and an upgraded industrial backdrop. All 109 sprite
textures export at 4x original canvas resolution. This is an AAA-inspired art
finish within the original 2D renderer; gameplay and hitboxes are unchanged.

The cinematic pack lives separately in assets/game/cinematic. The previous
supersuperhd files stay available as fallbacks and remain the basis for Cel
Shaded, so that option and all other themes retain their existing artwork.

Atlas sources are in source/cinematic_art. Re-export using
`python source/export_alpine_sprites.py --theme cinematic --atlases source/cinematic_art`.
Validate using `python source/test_alpine_assets.py --theme cinematic`.
Windows/Proton live gameplay has not been tested in this environment.

## v1.5.1 train-base fix

Only the train rendering below the standing surface is extended to the bottom
of the playfield, with overscan for camera shake. The platform top, collision
positions, sprite artwork, graphics modes and side borders are unchanged.
Geometry checks cover the actual train layer assets; live Windows/Proton
rendering has not been tested in this environment.

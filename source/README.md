# Native Windows source

This directory contains the C++ source corresponding to the final Windows build
with Rock music, louder music/announcements, and the spoken winner cue.

Build from the repository root so CMake also copies assets beside the executable:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
```

Use a Windows C++ compiler and SDK. The project requires CMake 3.20+ and C++17,
and links OpenGL and Windows system libraries. A MinGW-w64 cross-build is also
possible; the packaged MinGW executable additionally needs libwinpthread-1.dll.

Run DeadBabiesXNative.exe directly. No command launcher is needed. Keep game/
and music3.ogg, static-reactor.ogg, acid-house.ogg, and rock.ogg beside the EXE.

NativeSelfTest, audio-mixer self-tests, and deterministic gameplay fixtures are
implemented in the source. The distributed executable supports --self-test and
--audio-self-test; results are written to native.log.

Original source and attribution are retained in original/readme.txt.

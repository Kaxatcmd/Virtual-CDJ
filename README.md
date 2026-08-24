# Virtual CDJ

A Windows-based experimental software DJ deck designed to work alongside a
physical DJ setup.

The project turns a Windows laptop into a single virtual CDJ, using the
laptop's audio output as an additional stereo source connected to a
Behringer DDM4000 mixer.

## Hardware Setup

Current setup:

- Behringer DDM4000 DJ mixer
- Citronic MPCD-S3 CDJ
- Windows laptop
- Laptop integrated audio output
- 3.5 mm → RCA cable

Conceptually:

    Citronic CDJ ─────────→ DDM4000 Channel 1

    Windows Laptop
        Virtual CDJ
             │
             └────────────→ DDM4000 Channel 2


## Project Goal

The application should behave like a compact software CDJ rather than a
general-purpose music player.

The user should eventually be able to:

- Load local audio files
- Play and pause tracks
- Control pitch
- Pitch bend
- Set and recall cue points
- Display BPM
- Display an interactive waveform
- Create loops
- Use hot cues
- Simulate a virtual platter
- Perform turntable-style scratch behavior

Keyboard control is a first-class feature.

The essential deck workflow should be possible without relying on the mouse.

## Development Philosophy

The project will be developed incrementally.

The first milestone is deliberately simple:

    Local audio file
          ↓
    Audio engine
          ↓
    Windows audio output
          ↓
    3.5 mm → RCA
          ↓
    Behringer DDM4000

Once reliable playback is established, the project will progressively add
DJ-oriented functionality.

Do not attempt to implement the entire feature set at once.

## Documentation

- `ARCHITECTURE.md` — technical architecture, design decisions and roadmap.
- `README.md` — project overview and quick orientation.

Before implementing significant architectural changes, read
`ARCHITECTURE.md`.

## Current Status

First working vertical slice implemented: a compact single-deck CDJ-style
Windows application that loads a real local audio file (WAV/FLAC/OGG/AIFF,
plus MP3 via Windows Media Foundation) and plays it through the Windows
default audio output, with keyboard-first transport, pitch, cue, loop, hot
cues, an async waveform + best-effort BPM display, and a jog-style virtual
platter. See `ARCHITECTURE.md` section 22 for the technology decisions behind
this implementation, and section 21 for what is still a simplification (pitch
changes tempo+pitch together, BPM detection is a heuristic, scratch has no
inertia/DSP physics yet).

## Build Requirements

- Windows 10/11
- Visual Studio 2022 (Build Tools or full IDE) with the **Desktop development
  with C++** workload, and a Windows 10/11 SDK
- CMake 3.22+
- Git (only needed if you re-clone `libs/JUCE` yourself; it is already
  vendored in this repository)

JUCE 8.0.15 is vendored under `libs/JUCE` so the project builds fully
offline — no package manager or network access is required at configure
time.

## Building

From a shell with CMake and MSVC on `PATH` (e.g. a "Developer Command Prompt
for VS 2022", or a normal shell after installing the Build Tools):

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug --target VirtualCDJ
```

The application binary is produced at:

```
build\src\VirtualCDJ_artefacts\Debug\Virtual CDJ.exe
```

Use `--config Release` for an optimized build (recommended for actually
using the deck, since Debug JUCE builds have extra runtime checks).

### Building and running the tests

```powershell
cmake --build build --config Debug --target VirtualCDJTests
build\tests\Debug\VirtualCDJTests.exe
```

Tests cover deck/domain logic only (pitch math, cue/loop/hot-cue state, seek
clamping, command mapping, state transitions) via an in-memory fake audio
engine — no real audio device is required to run them. Alternatively, once
configured, `ctest --test-dir build -C Debug` runs the same suite.

### Opening in an IDE

- **Visual Studio**: File → Open → Folder... on the repository root (or open
  the generated `build\VirtualCDJ.sln` after configuring above); Visual
  Studio's CMake integration will pick up `CMakeLists.txt` directly.
- **VS Code**: open the folder with the CMake Tools extension installed; it
  will detect `CMakeLists.txt` and offer to configure/build/debug the
  `VirtualCDJ` target directly.

### Running

Launch `Virtual CDJ.exe` directly, or pass an audio file path as the first
command-line argument to load it automatically on startup (handy for testing
or an OS "Open with" association):

```powershell
"build\src\VirtualCDJ_artefacts\Debug\Virtual CDJ.exe" "C:\path\to\track.wav"
```

Use **Load Track...** (or drag-and-drop is not yet implemented) to load a
file from the running application. See the on-screen keyboard hint strip, or
`ARCHITECTURE.md` section 5, for the full keyboard mapping (Space = play/
pause, C/Q = cue/return-to-cue, I/O/L = loop in/out/toggle, arrows = pitch
bend/nudge, R = reset pitch, 1–8 = hot cues, S = scratch mode, A/D = platter
jog, Esc = stop).
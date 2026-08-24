# Virtual CDJ — Architecture & Project Brief

## 1. Project Overview

This project is a small but sophisticated software DJ deck designed to run on a Windows laptop and act as a **virtual CDJ / turntable-style deck** alongside a physical DJ setup.

The application must load audio files from the local filesystem and reproduce them through the laptop's audio output, which is connected to a DJ mixer using a 3.5 mm mini-jack → RCA cable.

The physical setup is:

    ┌──────────────────────────────┐
    │      Behringer DDM4000      │
    │                              │
    │  Channel 1 ← Citronic CDJ   │
    │  Channel 2 ← Laptop         │
    └──────────────────────────────┘
                    ↑
                    │ RCA / 3.5mm
                    │
             ┌──────┴───────┐
             │ Windows      │
             │ Laptop       │
             │              │
             │ Virtual CDJ  │
             └──────────────┘

The laptop software is intended to behave conceptually like a single additional
CDJ connected to the mixer.

The goal is NOT to build a full DJ application comparable to Rekordbox,
Serato, Traktor, etc.

The goal is to build a focused, technically interesting virtual deck that
can realistically be used alongside the physical Citronic CDJ.

---

# 2. Hardware Context

Current hardware:

- Behringer DDM4000 DJ mixer
- Citronic MPCD-S3 CDJ
- Windows laptop
- Laptop integrated sound card
- 3.5 mm headphone/audio output
- 3.5 mm → RCA cable

The software should initially target a **single stereo audio output** from
the laptop.

The mixer handles the physical mixing.

The software does NOT need to implement:

- software mixer
- EQ mixer
- crossfader
- channel faders
- effects rack
- multiple audio outputs
- external MIDI controller support

Those features may be considered future extensions but are explicitly
out of scope for the initial project.

---

# 3. Core Concept

The application should behave like a simplified software CDJ.

The user should be able to:

1. Browse/load a local audio file.
2. Play and pause the track.
3. Stop/reposition playback.
4. Set and recall a cue point.
5. Adjust playback pitch.
6. Temporarily pitch-bend the track.
7. Display BPM.
8. Display the audio waveform.
9. Navigate through the waveform.
10. Create loop-in / loop-out points.
11. Enable/disable looping.
12. Create hot cues.
13. Eventually simulate turntable-style scratching.

The application should prioritize **real-time control and tactile behavior**
over visual complexity.

---

# 4. Design Philosophy

The application should follow this principle:

> If the mouse disappeared, the user should still be able to perform the
> essential functions of the deck using the keyboard.

Keyboard control is therefore a first-class feature, not an accessibility
afterthought.

The UI should visually expose the keyboard shortcuts so that the user can
learn and operate the deck without constantly reaching for the mouse.

The mouse remains useful for:

- selecting tracks
- navigating the waveform
- positioning the playhead
- interacting with UI controls
- manipulating the virtual platter

But the essential DJ workflow must remain keyboard-driven.

---

# 5. Initial Keyboard Control Scheme

The initial keyboard mapping should be implemented from the beginning.

This mapping may evolve, but the architecture must allow shortcuts to be
changed without modifying the audio engine.

Suggested initial mapping:

| Key | Function |
|---|---|
| `Space` | Play / Pause |
| `C` | Cue |
| `I` | Loop In |
| `O` | Loop Out |
| `L` | Loop On / Off |
| `R` | Reset pitch |
| `←` | Temporary pitch bend down |
| `→` | Temporary pitch bend up |
| `↑` | Increase pitch |
| `↓` | Decrease pitch |
| `1` - `8` | Hot Cues 1-8 |
| `S` | Toggle scratch mode |
| `A` / `D` | Virtual platter movement |
| `Q` | Return to cue |
| `Esc` | Stop |

Important:

Keyboard input must distinguish between:

- key pressed
- key held
- key released

This is especially important for pitch bend and scratch simulation.

For example:

    RIGHT ARROW pressed
        ↓
    pitch bend +X%
        ↓
    RIGHT ARROW held
        ↓
    maintain temporary bend
        ↓
    RIGHT ARROW released
        ↓
    smoothly return to normal pitch

The exact behavior and percentages should be configurable.

---

# 6. High-Level Architecture

The application should be divided into independent layers.

Recommended architecture:

    ┌─────────────────────────────────────┐
    │              UI Layer               │
    │                                     │
    │ Waveform / BPM / Pitch / Cue / UI   │
    └──────────────────┬──────────────────┘
                       │
                       ▼
    ┌─────────────────────────────────────┐
    │          Deck Controller             │
    │                                     │
    │ Play / Pause / Cue / Loop / Pitch   │
    │ Hot Cues / Navigation / State       │
    └───────────────┬─────────┬───────────┘
                    │         │
                    ▼         ▼
          ┌──────────────┐  ┌──────────────┐
          │ Audio Engine │  │ Track        │
          │              │  │ Analysis     │
          │ Playback     │  │              │
          │ Pitch        │  │ BPM          │
          │ Seek         │  │ Waveform     │
          │ Loop         │  │ Beatgrid     │
          └──────┬───────┘  └──────────────┘
                 │
                 ▼
          ┌──────────────┐
          │ Audio Output │
          │ Windows      │
          └──────────────┘


---

# 7. Main Components

## 7.1 Audio Engine

Responsible exclusively for real-time audio behavior.

Responsibilities:

- decode audio
- playback
- pause
- stop
- seek
- report current playback position
- control playback rate
- pitch shifting / time stretching
- loop playback
- eventually scratch playback

The Audio Engine should NOT know anything about:

- keyboard shortcuts
- UI buttons
- waveform rendering
- file browser
- visual state

It should expose a clean API to the Deck Controller.

Example conceptual API:

    play()
    pause()
    stop()
    seek(position)
    set_pitch(value)
    pitch_bend(amount)
    set_loop(start, end)
    enable_loop()
    disable_loop()
    get_position()
    get_duration()


---

# 7.2 Deck Controller

The Deck Controller represents the logical state of the virtual CDJ.

It coordinates:

- Audio Engine
- Track metadata
- Cue points
- Loop state
- Pitch state
- Hot cues
- playback state

Example state:

    DeckState
        track
        playing
        position
        bpm
        pitch
        cue_point
        loop_in
        loop_out
        loop_enabled
        hot_cues[]
        scratch_mode


---

# 7.3 Input / Keyboard Controller

Keyboard handling must be isolated from the Deck Controller.

The keyboard layer translates physical keyboard events into logical
deck commands.

Example:

    SPACE
      ↓
    PlayPauseCommand
      ↓
    DeckController
      ↓
    AudioEngine


This allows keyboard mappings to be changed later without touching the
audio system.

Future controllers such as MIDI or HID could eventually use the same
command interface.

---

# 7.4 Track Manager

Responsible for local files.

Initial responsibilities:

- open local audio files
- validate supported formats
- read metadata
- provide track duration
- pass audio data to the Audio Engine
- manage recently loaded tracks

Initial file support should prioritize common formats such as:

- WAV
- MP3
- FLAC

Additional formats can be added later.

---

# 7.5 Track Analysis

Analysis should happen separately from real-time playback.

Responsibilities:

- BPM detection
- waveform generation
- beat detection
- beatgrid generation

Analysis should ideally happen asynchronously so that loading a large
track does not freeze the UI.

Generated analysis data should be cached locally.

Example:

    track.wav
       │
       ├── waveform
       ├── BPM
       └── beatgrid

On subsequent loads, cached analysis should be reused where possible.


---

# 7.6 Waveform Renderer

The waveform is a visual representation of the track.

It should display:

- complete track waveform
- current playhead
- cue point
- loop in
- loop out
- hot cue markers
- beatgrid (when available)

The waveform renderer must not control audio directly.

Clicking the waveform should send a seek command to the Deck Controller.

---

# 7.7 UI Layer

The UI should visually resemble a compact CDJ rather than a DAW.

Core information:

    Track name
    BPM
    Pitch %
    Current time
    Total duration
    Waveform
    Cue point
    Loop state
    Playback state
    Hot cues

The interface should avoid unnecessary complexity.

---

# 8. Pitch System

Pitch is a core feature.

The application must distinguish between:

### Permanent Pitch

Example:

    Pitch = +2.00%

This changes the effective playback tempo.

### Temporary Pitch Bend

Example:

    RIGHT ARROW held
        → temporarily speed up playback

    LEFT ARROW held
        → temporarily slow down playback

When released, the engine returns to the permanent pitch value.

The architecture should treat these as separate values:

    base_pitch
    pitch_bend

Effective playback state:

    effective_pitch = base_pitch + pitch_bend

The implementation should eventually support pitch shifting/time stretching
so that tempo changes do not necessarily change musical pitch.

---

# 9. Cue System

The deck should support a primary cue point.

Basic behavior:

    C
      ↓
    set cue at current position

When pressing `C` again, behavior should eventually emulate a CDJ-style
cue workflow.

The exact semantics should be configurable during implementation.

At minimum:

- set cue
- return to cue
- preview from cue
- stop/return behavior

---

# 10. Loop System

The first loop implementation should be time-position based.

Example:

    LOOP IN
        ↓
    01:32.420

    LOOP OUT
        ↓
    01:40.800

    LOOP ENABLED

The engine repeatedly plays:

    01:32.420 → 01:40.800

Only after reliable manual loops exist should quantized beat-based loops
be introduced.

Future loop features:

- 1 beat
- 2 beats
- 4 beats
- 8 beats
- 16 beats
- half loop
- double loop
- beat-quantized loop


---

# 11. BPM and Beatgrid

BPM detection is an analysis feature, not a real-time playback feature.

Example:

    Track BPM: 128.04

The system may later generate:

    |   |   |   |   |   |   |   |
    1   2   3   4   1   2   3   4

The beatgrid can then support:

- quantized loops
- beat-aware navigation
- future sync features

Important:

Automatic BPM detection must never be treated as infallible.

The architecture should eventually allow manual BPM correction.


---

# 12. Scratch Simulation

Scratch is a later-stage feature.

Do NOT implement scratch during the initial playback architecture.

A scratch engine should conceptually behave more like a virtual turntable
than a simple playback-rate controller.

Concept:

    virtual platter position
            ↓
    audio playback position

Forward platter movement:

    audio position → → →

Backward platter movement:

    audio position ← ← ←

The speed and direction of platter movement determine the resulting
audio movement.

Scratch should eventually support:

- forward movement
- backward movement
- stopping the platter
- rapid direction changes
- scratch mode
- keyboard simulation
- mouse/platter interaction

The scratch engine must remain independent from the normal pitch system.


---

# 13. Virtual Platter

The UI may eventually contain a virtual platter.

The platter should be able to receive:

- mouse movement
- keyboard input
- potentially MIDI/HID input in the future

Example:

    A / D
      ↓
    virtual platter movement
      ↓
    Scratch Engine
      ↓
    Audio Engine


---

# 14. Real-Time Audio Requirements

Audio playback must remain responsive.

Avoid performing expensive operations inside the real-time audio callback.

Do NOT perform:

- BPM analysis
- waveform generation
- file scanning
- UI rendering
- disk-heavy operations

inside the real-time audio path.

Real-time audio processing should remain deterministic and lightweight.

If the chosen language/library cannot provide sufficiently reliable
low-latency audio behavior for scratch and pitch manipulation, the audio
engine should be isolated so that it can later be replaced with a
lower-level implementation without rewriting the entire application.

---

# 15. Persistence

The application should eventually remember:

- cue points
- hot cues
- loops
- BPM corrections
- beatgrid corrections
- waveform cache

This data should be stored separately from the original audio files.

Never modify the original audio files.

Example conceptual structure:

    /library
        track.wav

    /analysis
        track.waveform
        track.analysis

    /metadata
        track.json


---

# 16. Development Phases

## Phase 1 — Minimal Audio Deck

Implement:

- application startup
- local file selection
- audio playback
- pause
- stop
- seek
- current position
- duration
- audio output selection

No waveform yet.

---

## Phase 2 — Keyboard-Controlled Deck

Implement:

- Space → Play/Pause
- C → Cue
- Q → Return to Cue
- R → Reset pitch
- Arrow keys → Pitch Bend
- configurable keyboard mappings

The deck should already be usable without a mouse.

---

## Phase 3 — Pitch Control

Implement:

- base pitch
- pitch range
- temporary pitch bend
- reset pitch
- UI pitch indicator

Target behavior should feel similar to a physical DJ deck.

---

## Phase 4 — Waveform

Implement:

- waveform generation
- waveform cache
- waveform display
- playhead
- click-to-seek
- cue marker

---

## Phase 5 — Cue & Loop

Implement:

- cue point
- loop in
- loop out
- loop enable/disable
- keyboard controls
- visual loop markers

---

## Phase 6 — BPM Analysis

Implement:

- BPM detection
- BPM display
- manual BPM adjustment
- beatgrid generation

---

## Phase 7 — Hot Cues

Implement:

- 1–8 hot cues
- keyboard access
- visual markers
- cue persistence

---

## Phase 8 — Virtual Platter

Implement:

- visual platter
- mouse interaction
- keyboard interaction
- platter position/state

---

## Phase 9 — Scratch Engine

Implement:

- platter-based playback
- forward/backward movement
- scratch mode
- rapid direction changes
- integration with virtual platter

This is expected to be one of the most technically demanding stages.

---

# 17. Non-Goals for Version 1

Do NOT implement initially:

- two software decks
- automatic beat sync
- software mixer
- effects
- streaming services
- music library management
- online metadata
- MIDI
- DVS
- recording
- cloud synchronization
- mobile support

These features distract from the core objective.

---

# 18. Engineering Principles

### Principle 1 — Audio Engine Independence

The audio engine must not depend on the UI.

### Principle 2 — Input Independence

Keyboard commands must be translated into generic deck commands.

### Principle 3 — Real-Time Safety

Heavy processing must never happen in the real-time audio path.

### Principle 4 — Incremental Development

Each phase must produce a usable application.

### Principle 5 — Hardware-Oriented UX

The application should behave like a physical deck rather than a generic
audio player.

### Principle 6 — Keyboard First

Essential deck operations must be available without a mouse.

### Principle 7 — Replaceable Components

Audio libraries, waveform rendering and analysis components should be
replaceable without rewriting the whole application.

---

# 19. Definition of Success

Version 1 is successful when the user can:

1. Load a local techno track.
2. Connect the laptop to the Behringer DDM4000.
3. Select the laptop audio output.
4. Press Space to play.
5. Hear the track through the mixer.
6. Adjust pitch using the keyboard.
7. Temporarily pitch-bend using the arrow keys.
8. Set and return to a cue point.
9. See the current playback position.
10. Use the system without depending on the mouse.

The final long-term goal is:

> The laptop should feel sufficiently like a physical CDJ that the user
> can place it beside the Citronic and actually use both as part of a DJ
> setup.

---

# 20. Instructions for AI Coding Agents

When modifying this project:

1. Read this document before making architectural changes.
2. Do not implement future phases prematurely.
3. Do not couple UI code directly to the audio engine.
4. Do not introduce global keyboard handlers that bypass the input layer.
5. Do not perform expensive analysis inside the real-time audio callback.
6. Prefer small, testable components.
7. Keep audio behavior deterministic.
8. Preserve the possibility of replacing the audio backend.
9. Update this architecture document when a major architectural decision
   changes.
10. Explain significant technical trade-offs before introducing complex
    dependencies.

When implementing a feature, identify which architectural layer owns it.

For example:

    Keyboard shortcut
        → Input Layer

    Play command
        → Deck Controller

    Actual audio playback
        → Audio Engine

    BPM detection
        → Track Analysis

    Waveform visualization
        → UI / Waveform Renderer

Never solve an architectural problem by simply adding logic to the UI.

---

# 21. Current Priority

The immediate priority is NOT to build the complete virtual CDJ.

The immediate priority is to establish a reliable foundation:

    Windows
      ↓
    Audio file
      ↓
    Audio Engine
      ↓
    Laptop audio output
      ↓
    3.5mm → RCA
      ↓
    Behringer DDM4000
      ↓
    Physical sound system

Once this works reliably, implement keyboard control and pitch.

Everything else builds on top of that foundation.
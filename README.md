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

Early development / architecture stage.

No implementation should be assumed to exist yet.
#pragma once

#include "../audio/IAudioEngine.h"

namespace vcdj
{
    /** Minimal virtual-platter -> audio-position bridge (ARCHITECTURE.md
        sections 12/13).

        This intentionally implements the *correct conceptual model* -

            platter movement -> virtual playback position -> audio engine

        - without the momentum/inertia physics of a real scratch DSP. Per
        ARCHITECTURE.md section 12 ("do not fake a finished scratch system"),
        a full scratch engine (audio-thread-accurate sub-block scrubbing,
        vinyl-brake physics, direction-reversal click suppression) is left as
        a documented future replacement behind this same interface; what is
        implemented here is deliberately kept honest and simple:

            - entering scratch mode pauses normal playback,
            - platter movement maps 1:1 to relative seeks,
            - leaving scratch mode resumes playback if it was previously
              playing.

        This is reliable (it can never leave the engine in a bad state) even
        though it is not a "real" scratch sound.
    */
    class ScratchEngine
    {
    public:
        explicit ScratchEngine (IAudioEngine& engineIn) : engine (engineIn) {}

        void setScratchMode (bool shouldBeActive)
        {
            if (shouldBeActive == active)
                return;

            active = shouldBeActive;

            if (active)
            {
                wasPlayingBeforeScratch = engine.isPlaying();
                engine.pause();
            }
            else if (wasPlayingBeforeScratch)
            {
                engine.play();
            }
        }

        bool isActive() const noexcept { return active; }

        /** Moves the virtual platter by a relative amount of playback time.
            Positive = forward. Safe to call rapidly (e.g. every input-poll
            tick while a jog key is held, or every mouse-drag event). */
        void jogBy (double deltaSeconds)
        {
            if (deltaSeconds == 0.0)
                return;

            engine.seek (engine.getPosition() + deltaSeconds);
        }

    private:
        IAudioEngine& engine;
        bool active = false;
        bool wasPlayingBeforeScratch = false;
    };
}

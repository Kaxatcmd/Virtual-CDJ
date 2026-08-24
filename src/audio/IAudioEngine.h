#pragma once

#include <juce_core/juce_core.h>

namespace vcdj
{
    /** Result of attempting to load a track. Loading can fail (unsupported format,
        corrupt file, missing file) and the rest of the application must degrade
        gracefully when that happens (see ARCHITECTURE.md section 19). */
    struct TrackLoadResult
    {
        bool success = false;
        juce::String errorMessage;

        static TrackLoadResult ok()                       { return { true, {} }; }
        static TrackLoadResult failure (juce::String msg)  { return { false, std::move (msg) }; }
    };

    /** The abstract boundary between the Deck Controller and real-time audio
        playback (ARCHITECTURE.md section 7.1).

        The Deck Controller only ever talks to this interface. The real
        implementation (AudioEngine) is backed by JUCE and a real audio device;
        a lightweight in-memory FakeAudioEngine is used in unit tests so that
        domain logic (cue/loop/pitch/hot-cue behaviour) can be tested without a
        sound card, exactly as required by ARCHITECTURE.md section 20 ("keep
        audio integration testing separate from domain logic").

        Every method here must be safe to call from the UI/controller thread.
        None of them may block on, or run inside, the real-time audio callback.
    */
    class IAudioEngine
    {
    public:
        virtual ~IAudioEngine() = default;

        virtual TrackLoadResult loadFile (const juce::File& file) = 0;
        virtual bool hasTrackLoaded() const = 0;

        virtual void play() = 0;
        virtual void pause() = 0;
        virtual void stop() = 0;
        virtual bool isPlaying() const = 0;

        /** Seeks to an absolute position. Implementations must clamp to
            [0, duration]. */
        virtual void seek (double positionSeconds) = 0;
        virtual double getPosition() const = 0;
        virtual double getDuration() const = 0;

        /** 1.0 = normal speed/pitch. This is a plain playback-rate multiplier
            (changes pitch and tempo together, like a turntable/CDJ without
            master-tempo); see ARCHITECTURE.md section 8 and the pitch section
            of the technology-decision notes in ARCHITECTURE.md. */
        virtual void setPlaybackRatio (double ratio) = 0;

        virtual void setLoopPoints (double startSeconds, double endSeconds) = 0;
        virtual void setLoopEnabled (bool enabled) = 0;
        virtual bool isLoopEnabled() const = 0;
    };
}

#pragma once

#include "DeckTypes.h"
#include "PitchEngine.h"
#include "ScratchEngine.h"
#include "../audio/IAudioEngine.h"
#include "../analysis/TrackAnalyzer.h"
#include "../input/Commands.h"

namespace vcdj
{
    /** The central domain object of the whole application (ARCHITECTURE.md
        section 7.2).

        DeckController owns the logical state of the virtual CDJ and is the
        *only* thing allowed to talk to IAudioEngine. The UI never touches the
        audio engine directly, and the input layer never touches it either -
        everything funnels through execute()/the named methods here.

        DeckController is constructed with a reference to an IAudioEngine, so
        production code wires it to the real JUCE-backed AudioEngine while
        unit tests wire it to a trivial in-memory FakeAudioEngine.
    */
    class DeckController
    {
    public:
        explicit DeckController (IAudioEngine& engineIn);

        //==============================================================================
        // Track loading
        TrackLoadResult loadTrack (const juce::File& file);
        bool hasTrackLoaded() const { return engine.hasTrackLoaded(); }
        juce::String getTrackName() const { return trackName; }

        //==============================================================================
        // Transport
        void togglePlayPause();
        void play();
        void pause();
        void stop();
        void seek (double positionSeconds);
        double getPosition() const { return engine.getPosition(); }
        double getDuration() const { return engine.getDuration(); }
        bool isPlaying() const { return engine.isPlaying(); }

        /** Clamps a requested seek position into the valid [0, duration]
            range. Exposed as a static pure function so it can be unit tested
            without any audio engine at all. */
        static double clampSeekPosition (double requested, double duration);

        //==============================================================================
        // Cue
        void setCue();
        void returnToCue();
        const CueState& getCueState() const { return cue; }

        //==============================================================================
        // Loop
        void loopIn();
        void loopOut();
        void toggleLoop();
        const LoopState& getLoopState() const { return loop; }

        //==============================================================================
        // Pitch
        void resetPitch();
        void nudgePitch (double deltaPercent);
        void beginPitchBend (bool positiveDirection);
        void endPitchBend();
        const PitchEngine& getPitchEngine() const { return pitch; }

        //==============================================================================
        // Hot cues
        void triggerHotCue (int slotIndex);
        const HotCueStore& getHotCueStore() const { return hotCues; }

        //==============================================================================
        // Scratch / virtual platter
        void toggleScratchMode();
        void setScratchActive (bool active);
        void jogPlatter (double deltaSeconds);
        bool isScratchModeActive() const { return scratch.isActive(); }

        //==============================================================================
        /** Single entry point for discrete commands coming from the input
            layer (ARCHITECTURE.md section 7.3 command-flow diagram). */
        void execute (CommandId command);

        //==============================================================================
        /** Advances time-based smoothing (pitch bend decay) and pushes the
            resulting effective playback ratio into the audio engine. Call
            regularly (e.g. every UI frame) from the message thread. */
        void tick (double deltaSeconds);

        //==============================================================================
        // Analysis (BPM + waveform), always optional/best-effort
        bool isAnalysisComplete() const { return analyzer.isComplete(); }
        bool hasValidBpm() const { return analyzer.hasValidBpm(); }
        double getBpm() const { return analyzer.getBpm(); }
        waveform::WaveformData getWaveform() const { return analyzer.getWaveformCopy(); }

        //==============================================================================
        DeckSnapshot getSnapshot() const;

    private:
        void syncLoopToEngine();

        IAudioEngine& engine;
        PitchEngine pitch;
        ScratchEngine scratch;
        CueState cue;
        LoopState loop;
        HotCueStore hotCues;
        TrackAnalyzer analyzer;

        juce::String trackName;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DeckController)
    };
}

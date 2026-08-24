#include "DeckController.h"

namespace vcdj
{
    DeckController::DeckController (IAudioEngine& engineIn)
        : engine (engineIn), scratch (engineIn)
    {
    }

    TrackLoadResult DeckController::loadTrack (const juce::File& file)
    {
        auto result = engine.loadFile (file);

        if (result.success)
        {
            trackName = file.getFileNameWithoutExtension();

            cue.clear();
            loop.clear();
            hotCues.clearAll();
            pitch.resetPitch();
            scratch.setScratchMode (false);

            // Fire-and-forget: analysis runs on its own thread and never
            // blocks or gates playback (ARCHITECTURE.md sections 8 and 11).
            analyzer.analyze (file);
        }

        return result;
    }

    //==============================================================================
    void DeckController::togglePlayPause()
    {
        if (engine.isPlaying())
            pause();
        else
            play();
    }

    void DeckController::play()  { engine.play(); }
    void DeckController::pause() { engine.pause(); }
    void DeckController::stop()  { engine.stop(); }

    double DeckController::clampSeekPosition (double requested, double duration)
    {
        return juce::jlimit (0.0, juce::jmax (0.0, duration), requested);
    }

    void DeckController::seek (double positionSeconds)
    {
        engine.seek (clampSeekPosition (positionSeconds, engine.getDuration()));
    }

    //==============================================================================
    void DeckController::setCue()
    {
        cue.setCue (getPosition());
    }

    void DeckController::returnToCue()
    {
        if (! cue.hasCue())
            cue.setCue (0.0);

        seek (cue.getPosition());
    }

    //==============================================================================
    void DeckController::syncLoopToEngine()
    {
        if (loop.isValid())
            engine.setLoopPoints (loop.getLoopIn(), loop.getLoopOut());

        // Re-validate: enabling only "sticks" if the current in/out points
        // actually form a valid loop.
        loop.setEnabled (loop.isEnabled());
        engine.setLoopEnabled (loop.isEnabled());
    }

    void DeckController::loopIn()
    {
        loop.setLoopIn (getPosition());
        syncLoopToEngine();
    }

    void DeckController::loopOut()
    {
        loop.setLoopOut (getPosition());
        syncLoopToEngine();
    }

    void DeckController::toggleLoop()
    {
        loop.toggle();
        syncLoopToEngine();
    }

    //==============================================================================
    void DeckController::resetPitch()                       { pitch.resetPitch(); }
    void DeckController::nudgePitch (double deltaPercent)    { pitch.nudgeBasePitch (deltaPercent); }
    void DeckController::beginPitchBend (bool positiveDirection) { pitch.beginBend (positiveDirection); }
    void DeckController::endPitchBend()                      { pitch.endBend(); }

    //==============================================================================
    void DeckController::triggerHotCue (int slotIndex)
    {
        auto result = hotCues.trigger (slotIndex, getPosition());

        if (result.action == HotCueStore::TriggerAction::jumpTo)
            seek (result.positionSeconds);
    }

    //==============================================================================
    void DeckController::toggleScratchMode()               { scratch.setScratchMode (! scratch.isActive()); }
    void DeckController::setScratchActive (bool active)    { scratch.setScratchMode (active); }
    void DeckController::jogPlatter (double deltaSeconds)   { scratch.jogBy (deltaSeconds); }

    //==============================================================================
    void DeckController::execute (CommandId command)
    {
        switch (command)
        {
            case CommandId::playPause:      togglePlayPause(); break;
            case CommandId::stop:           stop(); break;
            case CommandId::cue:            setCue(); break;
            case CommandId::returnToCue:    returnToCue(); break;
            case CommandId::loopIn:         loopIn(); break;
            case CommandId::loopOut:        loopOut(); break;
            case CommandId::loopToggle:     toggleLoop(); break;
            case CommandId::pitchReset:     resetPitch(); break;
            case CommandId::pitchNudgeUp:   nudgePitch (0.1); break;
            case CommandId::pitchNudgeDown: nudgePitch (-0.1); break;
            case CommandId::scratchToggle:  toggleScratchMode(); break;
            case CommandId::hotCue1: triggerHotCue (0); break;
            case CommandId::hotCue2: triggerHotCue (1); break;
            case CommandId::hotCue3: triggerHotCue (2); break;
            case CommandId::hotCue4: triggerHotCue (3); break;
            case CommandId::hotCue5: triggerHotCue (4); break;
            case CommandId::hotCue6: triggerHotCue (5); break;
            case CommandId::hotCue7: triggerHotCue (6); break;
            case CommandId::hotCue8: triggerHotCue (7); break;
        }
    }

    //==============================================================================
    void DeckController::tick (double deltaSeconds)
    {
        pitch.tick (deltaSeconds);
        engine.setPlaybackRatio (pitch.getEffectiveRatio());
    }

    //==============================================================================
    DeckSnapshot DeckController::getSnapshot() const
    {
        DeckSnapshot s;
        s.trackLoaded = hasTrackLoaded();
        s.trackName = trackName;
        s.playing = isPlaying();
        s.positionSeconds = getPosition();
        s.durationSeconds = getDuration();

        s.basePitchPercent = pitch.getBasePitchPercent();
        s.bendPercent = pitch.getBendPercent();
        s.effectiveRatio = pitch.getEffectiveRatio();

        s.hasCue = cue.hasCue();
        s.cuePositionSeconds = cue.getPosition();

        s.hasLoopIn = loop.hasLoopIn();
        s.hasLoopOut = loop.hasLoopOut();
        s.loopEnabled = loop.isEnabled();
        s.loopInSeconds = loop.getLoopIn();
        s.loopOutSeconds = loop.getLoopOut();

        for (int i = 0; i < HotCueStore::numSlots; ++i)
            s.hotCues[(size_t) i] = hotCues.get (i);

        s.scratchMode = scratch.isActive();

        s.bpmAnalysisComplete = analyzer.isComplete();
        s.bpm = analyzer.hasValidBpm() ? analyzer.getBpm() : 0.0;

        return s;
    }
}

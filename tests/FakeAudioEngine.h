#pragma once

#include "audio/IAudioEngine.h"

/** A trivial in-memory stand-in for AudioEngine, used only by unit tests so
    that DeckController's domain logic (cue/loop/pitch/hot-cue/seek-clamping
    behaviour) can be exercised deterministically without a sound card or a
    real audio file (ARCHITECTURE.md section 20: "keep audio integration
    testing separate from domain logic").
*/
class FakeAudioEngine : public vcdj::IAudioEngine
{
public:
    vcdj::TrackLoadResult loadFile (const juce::File&) override
    {
        loaded = true;
        position = 0.0;
        return vcdj::TrackLoadResult::ok();
    }

    bool hasTrackLoaded() const override { return loaded; }

    void play() override  { if (loaded) playing = true; }
    void pause() override { playing = false; }
    void stop() override  { playing = false; position = 0.0; }
    bool isPlaying() const override { return playing; }

    void seek (double positionSeconds) override
    {
        position = juce::jlimit (0.0, duration, positionSeconds);
    }

    double getPosition() const override { return position; }
    double getDuration() const override { return duration; }

    void setPlaybackRatio (double r) override { ratio = r; }
    double getPlaybackRatioForTest() const { return ratio; }

    void setLoopPoints (double startSeconds, double endSeconds) override
    {
        loopStart = startSeconds;
        loopEnd = endSeconds;
    }

    void setLoopEnabled (bool e) override { loopEnabledFlag = e; }
    bool isLoopEnabled() const override { return loopEnabledFlag; }

    void setDurationForTest (double newDuration) { duration = newDuration; }

private:
    bool loaded = false;
    bool playing = false;
    double position = 0.0;
    double duration = 180.0;
    double ratio = 1.0;
    double loopStart = 0.0;
    double loopEnd = 0.0;
    bool loopEnabledFlag = false;
};

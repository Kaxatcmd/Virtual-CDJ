#pragma once

#include "IAudioEngine.h"
#include "TrackAudioSource.h"
#include <juce_audio_utils/juce_audio_utils.h>

namespace vcdj
{
    /** JUCE-backed implementation of IAudioEngine.

        Owns the audio device, the decoder chain and the real-time playback
        graph:

            TrackAudioSource (decode + play/pause + manual loop)
                -> juce::ResamplingAudioSource (pitch/speed ratio)
                    -> juce::AudioSourcePlayer -> juce::AudioDeviceManager -> WASAPI

        Nothing in this class knows about the UI, keyboard input or the Deck
        Controller's domain state - it only exposes the transport primitives
        declared in IAudioEngine (ARCHITECTURE.md section 7.1).
    */
    class AudioEngine final : public IAudioEngine
    {
    public:
        AudioEngine();
        ~AudioEngine() override;

        TrackLoadResult loadFile (const juce::File& file) override;
        bool hasTrackLoaded() const override;

        void play() override;
        void pause() override;
        void stop() override;
        bool isPlaying() const override;

        void seek (double positionSeconds) override;
        double getPosition() const override;
        double getDuration() const override;

        void setPlaybackRatio (double ratio) override;

        void setLoopPoints (double startSeconds, double endSeconds) override;
        void setLoopEnabled (bool enabled) override;
        bool isLoopEnabled() const override;

    private:
        juce::AudioFormatManager formatManager;
        juce::AudioDeviceManager deviceManager;
        juce::AudioSourcePlayer sourcePlayer;
        juce::TimeSliceThread readAheadThread { "VCDJ Read-Ahead" };

        std::unique_ptr<TrackAudioSource> trackSource;
        std::unique_ptr<juce::ResamplingAudioSource> resamplingSource;

        std::atomic<double> sourceSampleRate { 44100.0 };
        std::atomic<double> durationSeconds { 0.0 };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioEngine)
    };
}

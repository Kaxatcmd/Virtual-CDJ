#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <atomic>

namespace vcdj
{
    /** A PositionableAudioSource that plays a decoded audio file and can loop a
        sample-accurate [loopStart, loopEnd) region.

        Real-time safety: the actual file reading happens on a background
        juce::TimeSliceThread via the JUCE-provided BufferingAudioSource, so the
        audio callback (getNextAudioBlock) only ever copies already-decoded
        samples out of RAM. Play/pause state and loop points are plain atomics
        so the UI/controller thread can update them without locking the audio
        thread (ARCHITECTURE.md section 15 "Real-Time Audio Requirements").
    */
    class TrackAudioSource final : public juce::PositionableAudioSource
    {
    public:
        TrackAudioSource (std::unique_ptr<juce::AudioFormatReaderSource> readerSource,
                           juce::TimeSliceThread& backgroundThread,
                           double sourceSampleRateIn);

        ~TrackAudioSource() override;

        //==============================================================================
        void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
        void releaseResources() override;
        void getNextAudioBlock (const juce::AudioSourceChannelInfo& info) override;

        //==============================================================================
        void setNextReadPosition (juce::int64 newPosition) override;
        juce::int64 getNextReadPosition() const override;
        juce::int64 getTotalLength() const override;

        /** We repurpose the standard isLooping()/setLooping() pair for our
            manual A/B loop, rather than a whole-file repeat. */
        bool isLooping() const override;
        void setLooping (bool shouldLoop) override;

        //==============================================================================
        void setPlaying (bool shouldPlay);
        bool isPlaying() const noexcept;

        /** Loop points expressed in source-file samples. endSample <= startSample
            means "no valid loop" and is treated as disabled even if setLooping(true)
            was called. */
        void setLoopPointsSamples (juce::int64 startSample, juce::int64 endSample);

        double getSourceSampleRate() const noexcept { return sourceSampleRate; }

    private:
        std::unique_ptr<juce::BufferingAudioSource> bufferingSource;
        double sourceSampleRate;

        std::atomic<bool> playing { false };
        std::atomic<bool> loopEnabled { false };
        std::atomic<juce::int64> loopStartSample { 0 };
        std::atomic<juce::int64> loopEndSample { -1 };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackAudioSource)
    };
}

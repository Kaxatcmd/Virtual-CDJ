#pragma once

#include "../waveform/WaveformData.h"
#include <juce_audio_formats/juce_audio_formats.h>
#include <atomic>

namespace vcdj
{
    /** Asynchronous, best-effort track analysis (ARCHITECTURE.md sections 7.5,
        8 and 11).

        Runs entirely on a background juce::Thread: reads the file with its own
        AudioFormatReader (completely separate from the one used for playback),
        extracts a fixed-resolution min/max waveform overview and estimates a
        single BPM value using a simple autocorrelation of a short-time-energy
        onset envelope.

        This is deliberately a "good enough" estimator, not a research-grade
        beat tracker: it is asynchronous, never blocks playback, and a failed
        or inaccurate estimate never prevents the track from loading/playing
        (the caller just sees isComplete() with bpm() == 0, or a wrong-ish BPM
        that the architecture already documents as eventually manually
        correctable).

        Results are cached to disk (keyed by path + size + modification time)
        under AppPaths::getAnalysisCacheDirectory() so re-loading the same
        track is instant and the original audio file is never touched.
    */
    class TrackAnalyzer : private juce::Thread
    {
    public:
        TrackAnalyzer();
        ~TrackAnalyzer() override;

        /** Starts analyzing (asynchronously). Cancels/waits for any previous
            in-flight analysis first. Safe to call from the message thread. */
        void analyze (const juce::File& file);

        bool isComplete() const { return complete.load (std::memory_order_acquire); }
        bool hasValidBpm() const { return bpmValid.load (std::memory_order_acquire); }
        double getBpm() const { return bpm.load (std::memory_order_acquire); }

        /** Thread-safe copy for the UI/waveform view to draw from. */
        waveform::WaveformData getWaveformCopy() const;

    private:
        void run() override;
        bool tryLoadFromCache (const juce::File& file, const juce::String& cacheKey);
        void saveToCache (const juce::File& file, const juce::String& cacheKey) const;

        juce::AudioFormatManager formatManager;

        mutable juce::CriticalSection waveformLock;
        waveform::WaveformData waveformData;

        std::atomic<double> bpm { 0.0 };
        std::atomic<bool> bpmValid { false };
        std::atomic<bool> complete { false };

        juce::File fileBeingAnalyzed;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackAnalyzer)
    };
}

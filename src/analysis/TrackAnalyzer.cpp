#include "TrackAnalyzer.h"
#include "../core/AppPaths.h"
#include <cmath>
#include <numeric>
#include <cstring>

namespace vcdj
{
    namespace
    {
        constexpr int numWaveformPeaks = 2000;
        constexpr int onsetFrameSize = 1024;
        constexpr double minDetectableBpm = 60.0;
        constexpr double maxDetectableBpm = 200.0;
        constexpr const char* cacheMagic = "VCDJC001";

        juce::String makeCacheKey (const juce::File& file)
        {
            const juce::String identity = file.getFullPathName()
                                           + "|" + juce::String ((juce::int64) file.getSize())
                                           + "|" + juce::String (file.getLastModificationTime().toMilliseconds());
            // A cryptographic hash isn't required here - we only need a short,
            // stable, filesystem-safe key derived from the file identity, so we
            // avoid pulling in the juce_cryptography module for this alone.
            return juce::String::toHexString (identity.hashCode64());
        }
    }

    TrackAnalyzer::TrackAnalyzer() : juce::Thread ("VCDJ Track Analyzer")
    {
        formatManager.registerBasicFormats();
    }

    TrackAnalyzer::~TrackAnalyzer()
    {
        stopThread (4000);
    }

    void TrackAnalyzer::analyze (const juce::File& file)
    {
        stopThread (2000);

        complete.store (false, std::memory_order_release);
        bpmValid.store (false, std::memory_order_release);
        bpm.store (0.0, std::memory_order_release);

        {
            const juce::ScopedLock sl (waveformLock);
            waveformData = {};
        }

        fileBeingAnalyzed = file;
        startThread (juce::Thread::Priority::low);
    }

    waveform::WaveformData TrackAnalyzer::getWaveformCopy() const
    {
        const juce::ScopedLock sl (waveformLock);
        return waveformData;
    }

    bool TrackAnalyzer::tryLoadFromCache (const juce::File& file, const juce::String& cacheKey)
    {
        juce::ignoreUnused (file);

        auto cacheFile = AppPaths::getAnalysisCacheDirectory().getChildFile (cacheKey + ".vcdjcache");

        if (! cacheFile.existsAsFile())
            return false;

        juce::FileInputStream in (cacheFile);

        if (! in.openedOk())
            return false;

        char magic[8] = {};
        in.read (magic, 8);

        if (std::memcmp (magic, cacheMagic, 8) != 0)
            return false;

        const double duration = in.readDouble();
        const double bpmValue = in.readDouble();
        const bool bpmIsValid = in.readByte() != 0;
        const int numPeaks = in.readInt();

        if (numPeaks < 0 || numPeaks > 1000000)
            return false;

        waveform::WaveformData loaded;
        loaded.durationSeconds = duration;
        loaded.peaks.resize ((size_t) numPeaks);

        for (int i = 0; i < numPeaks; ++i)
        {
            loaded.peaks[(size_t) i].minValue = in.readFloat();
            loaded.peaks[(size_t) i].maxValue = in.readFloat();
        }

        {
            const juce::ScopedLock sl (waveformLock);
            waveformData = std::move (loaded);
        }

        bpm.store (bpmValue, std::memory_order_release);
        bpmValid.store (bpmIsValid, std::memory_order_release);
        return true;
    }

    void TrackAnalyzer::saveToCache (const juce::File& file, const juce::String& cacheKey) const
    {
        juce::ignoreUnused (file);

        waveform::WaveformData snapshot;
        {
            const juce::ScopedLock sl (waveformLock);
            snapshot = waveformData;
        }

        auto cacheFile = AppPaths::getAnalysisCacheDirectory().getChildFile (cacheKey + ".vcdjcache");
        cacheFile.deleteFile();
        juce::FileOutputStream out (cacheFile);

        if (! out.openedOk())
            return;

        out.write (cacheMagic, 8);
        out.writeDouble (snapshot.durationSeconds);
        out.writeDouble (bpm.load (std::memory_order_acquire));
        out.writeByte (bpmValid.load (std::memory_order_acquire) ? 1 : 0);
        out.writeInt ((int) snapshot.peaks.size());

        for (auto& p : snapshot.peaks)
        {
            out.writeFloat (p.minValue);
            out.writeFloat (p.maxValue);
        }
    }

    void TrackAnalyzer::run()
    {
        const auto file = fileBeingAnalyzed;
        const auto cacheKey = makeCacheKey (file);

        if (tryLoadFromCache (file, cacheKey))
        {
            complete.store (true, std::memory_order_release);
            return;
        }

        std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));

        if (reader == nullptr || reader->lengthInSamples <= 0 || reader->sampleRate <= 0.0)
        {
            // Analysis failure must never block playback - report "no data" and stop.
            complete.store (true, std::memory_order_release);
            return;
        }

        const juce::int64 totalLength = reader->lengthInSamples;
        const double sampleRate = reader->sampleRate;
        const double duration = (double) totalLength / sampleRate;

        const juce::int64 samplesPerPeak = juce::jmax<juce::int64> (1, totalLength / numWaveformPeaks);

        waveform::WaveformData newWaveform;
        newWaveform.durationSeconds = duration;
        newWaveform.peaks.reserve ((size_t) numWaveformPeaks + 1);

        std::vector<float> onsetEnvelope;
        onsetEnvelope.reserve ((size_t) (totalLength / onsetFrameSize) + 1);

        constexpr int chunkSize = 65536;
        juce::AudioBuffer<float> chunk (juce::jmax (1, (int) reader->numChannels), chunkSize);

        juce::int64 samplesReadTotal = 0;
        float curMin = 0.0f, curMax = 0.0f;
        juce::int64 samplesInCurrentPeak = 0;

        double frameEnergyAccum = 0.0;
        int samplesInFrame = 0;

        while (samplesReadTotal < totalLength)
        {
            if (threadShouldExit())
                return;

            const int samplesThisChunk = (int) juce::jmin<juce::int64> (chunkSize, totalLength - samplesReadTotal);
            reader->read (&chunk, 0, samplesThisChunk, samplesReadTotal, true, true);

            for (int i = 0; i < samplesThisChunk; ++i)
            {
                float sample = chunk.getSample (0, i);

                if (chunk.getNumChannels() > 1)
                    sample = 0.5f * (sample + chunk.getSample (1, i));

                curMin = juce::jmin (curMin, sample);
                curMax = juce::jmax (curMax, sample);
                ++samplesInCurrentPeak;

                if (samplesInCurrentPeak >= samplesPerPeak)
                {
                    newWaveform.peaks.push_back ({ curMin, curMax });
                    curMin = 0.0f;
                    curMax = 0.0f;
                    samplesInCurrentPeak = 0;
                }

                frameEnergyAccum += (double) sample * (double) sample;
                ++samplesInFrame;

                if (samplesInFrame >= onsetFrameSize)
                {
                    onsetEnvelope.push_back ((float) std::sqrt (frameEnergyAccum / samplesInFrame));
                    frameEnergyAccum = 0.0;
                    samplesInFrame = 0;
                }
            }

            samplesReadTotal += samplesThisChunk;
        }

        if (samplesInCurrentPeak > 0)
            newWaveform.peaks.push_back ({ curMin, curMax });

        {
            const juce::ScopedLock sl (waveformLock);
            waveformData = std::move (newWaveform);
        }

        // --- Best-effort BPM estimate: autocorrelation of the onset envelope ---
        double estimatedBpm = 0.0;
        bool valid = false;

        if (onsetEnvelope.size() > 8)
        {
            const double frameRate = sampleRate / (double) onsetFrameSize;
            const double meanEnv = std::accumulate (onsetEnvelope.begin(), onsetEnvelope.end(), 0.0)
                                    / (double) onsetEnvelope.size();

            std::vector<float> centered (onsetEnvelope.size());
            for (size_t i = 0; i < onsetEnvelope.size(); ++i)
                centered[i] = onsetEnvelope[i] - (float) meanEnv;

            const int minLag = juce::jmax (1, (int) std::round (frameRate * 60.0 / maxDetectableBpm));
            const int maxLag = juce::jmin ((int) centered.size() - 1, (int) std::round (frameRate * 60.0 / minDetectableBpm));

            double bestScore = -1.0;
            int bestLag = -1;

            for (int lag = minLag; lag <= maxLag; ++lag)
            {
                if (threadShouldExit())
                    return;

                const int n = (int) centered.size() - lag;
                if (n <= 0)
                    continue;

                double sum = 0.0;
                for (int i = 0; i < n; ++i)
                    sum += (double) centered[(size_t) i] * (double) centered[(size_t) (i + lag)];

                const double score = sum / (double) n;

                if (score > bestScore)
                {
                    bestScore = score;
                    bestLag = lag;
                }
            }

            if (bestLag > 0 && bestScore > 0.0)
            {
                estimatedBpm = 60.0 * frameRate / (double) bestLag;

                // Autocorrelation tempo estimators very commonly lock onto half or
                // double the true tempo; fold the result into a "typical" DJ range.
                while (estimatedBpm < 80.0)  estimatedBpm *= 2.0;
                while (estimatedBpm > 175.0) estimatedBpm /= 2.0;

                valid = true;
            }
        }

        bpm.store (estimatedBpm, std::memory_order_release);
        bpmValid.store (valid, std::memory_order_release);
        complete.store (true, std::memory_order_release);

        saveToCache (file, cacheKey);
    }
}

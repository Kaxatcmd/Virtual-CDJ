#include "AudioEngine.h"

namespace vcdj
{
    AudioEngine::AudioEngine()
    {
        formatManager.registerBasicFormats();
        readAheadThread.startThread (juce::Thread::Priority::high);

        // Single stereo output only - no inputs are required (ARCHITECTURE.md
        // section 2 explicitly excludes software mixing / external inputs).
        auto error = deviceManager.initialiseWithDefaultDevices (0, 2);

        if (error.isNotEmpty())
            juce::Logger::writeToLog ("AudioEngine: failed to open an audio output device - " + error);

        deviceManager.addAudioCallback (&sourcePlayer);
    }

    AudioEngine::~AudioEngine()
    {
        deviceManager.removeAudioCallback (&sourcePlayer);
        sourcePlayer.setSource (nullptr);
        resamplingSource.reset();
        trackSource.reset();
        readAheadThread.stopThread (2000);
    }

    TrackLoadResult AudioEngine::loadFile (const juce::File& file)
    {
        if (! file.existsAsFile())
            return TrackLoadResult::failure ("File does not exist: " + file.getFullPathName());

        std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));

        if (reader == nullptr)
            return TrackLoadResult::failure ("Unsupported or corrupt audio file: " + file.getFileName());

        if (reader->lengthInSamples <= 0 || reader->sampleRate <= 0.0)
            return TrackLoadResult::failure ("Audio file contains no playable audio: " + file.getFileName());

        const double newDuration = (double) reader->lengthInSamples / reader->sampleRate;
        const double newSampleRate = reader->sampleRate;

        auto newReaderSource = std::make_unique<juce::AudioFormatReaderSource> (reader.release(), true);
        auto newTrackSource = std::make_unique<TrackAudioSource> (std::move (newReaderSource),
                                                                    readAheadThread,
                                                                    newSampleRate);
        auto newResamplingSource = std::make_unique<juce::ResamplingAudioSource> (newTrackSource.get(), false, 2);

        // Detach the player from whatever is currently playing before swapping
        // the graph, then hand it the new chain. AudioSourcePlayer::setSource()
        // is safe to call from the message thread while the audio callback is
        // running (it takes an internal lock only around the pointer swap).
        sourcePlayer.setSource (nullptr);

        trackSource = std::move (newTrackSource);
        resamplingSource = std::move (newResamplingSource);
        sourceSampleRate.store (newSampleRate, std::memory_order_release);
        durationSeconds.store (newDuration, std::memory_order_release);

        sourcePlayer.setSource (resamplingSource.get());

        return TrackLoadResult::ok();
    }

    bool AudioEngine::hasTrackLoaded() const
    {
        return trackSource != nullptr;
    }

    void AudioEngine::play()
    {
        if (trackSource != nullptr)
            trackSource->setPlaying (true);
    }

    void AudioEngine::pause()
    {
        if (trackSource != nullptr)
            trackSource->setPlaying (false);
    }

    void AudioEngine::stop()
    {
        if (trackSource != nullptr)
        {
            trackSource->setPlaying (false);
            seek (0.0);
        }
    }

    bool AudioEngine::isPlaying() const
    {
        return trackSource != nullptr && trackSource->isPlaying();
    }

    void AudioEngine::seek (double positionSeconds)
    {
        if (trackSource == nullptr)
            return;

        const auto duration = durationSeconds.load (std::memory_order_acquire);
        const auto clamped = juce::jlimit (0.0, duration, positionSeconds);
        const auto sr = sourceSampleRate.load (std::memory_order_acquire);

        trackSource->setNextReadPosition ((juce::int64) (clamped * sr));
    }

    double AudioEngine::getPosition() const
    {
        if (trackSource == nullptr)
            return 0.0;

        const auto sr = sourceSampleRate.load (std::memory_order_acquire);
        return sr > 0.0 ? (double) trackSource->getNextReadPosition() / sr : 0.0;
    }

    double AudioEngine::getDuration() const
    {
        return durationSeconds.load (std::memory_order_acquire);
    }

    void AudioEngine::setPlaybackRatio (double ratio)
    {
        if (resamplingSource != nullptr)
            resamplingSource->setResamplingRatio (juce::jmax (0.01, ratio));
    }

    void AudioEngine::setLoopPoints (double startSeconds, double endSeconds)
    {
        if (trackSource == nullptr)
            return;

        const auto sr = sourceSampleRate.load (std::memory_order_acquire);
        trackSource->setLoopPointsSamples ((juce::int64) (startSeconds * sr), (juce::int64) (endSeconds * sr));
    }

    void AudioEngine::setLoopEnabled (bool enabled)
    {
        if (trackSource != nullptr)
            trackSource->setLooping (enabled);
    }

    bool AudioEngine::isLoopEnabled() const
    {
        return trackSource != nullptr && trackSource->isLooping();
    }
}

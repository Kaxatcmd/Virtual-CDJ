#include "TrackAudioSource.h"

namespace vcdj
{
    TrackAudioSource::TrackAudioSource (std::unique_ptr<juce::AudioFormatReaderSource> readerSource,
                                         juce::TimeSliceThread& backgroundThread,
                                         double sourceSampleRateIn)
        : sourceSampleRate (sourceSampleRateIn)
    {
        constexpr int readAheadBufferSamples = 65536;

        bufferingSource = std::make_unique<juce::BufferingAudioSource> (
            readerSource.release(), backgroundThread, true, readAheadBufferSamples, 2, true);
    }

    TrackAudioSource::~TrackAudioSource() = default;

    void TrackAudioSource::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
    {
        bufferingSource->prepareToPlay (samplesPerBlockExpected, sampleRate);
    }

    void TrackAudioSource::releaseResources()
    {
        bufferingSource->releaseResources();
    }

    void TrackAudioSource::getNextAudioBlock (const juce::AudioSourceChannelInfo& info)
    {
        if (! playing.load (std::memory_order_acquire))
        {
            info.clearActiveBufferRegion();
            return;
        }

        if (loopEnabled.load (std::memory_order_acquire))
        {
            const auto loopStart = loopStartSample.load (std::memory_order_acquire);
            const auto loopEnd   = loopEndSample.load (std::memory_order_acquire);

            if (loopEnd > loopStart)
            {
                auto currentPos = bufferingSource->getNextReadPosition();

                if (currentPos >= loopEnd || currentPos < loopStart)
                {
                    bufferingSource->setNextReadPosition (loopStart);
                    currentPos = loopStart;
                }

                const auto samplesUntilLoopEnd = (int) juce::jlimit<juce::int64> (
                    0, (juce::int64) info.numSamples, loopEnd - currentPos);

                if (samplesUntilLoopEnd < info.numSamples)
                {
                    if (samplesUntilLoopEnd > 0)
                    {
                        juce::AudioSourceChannelInfo firstPart (info.buffer, info.startSample, samplesUntilLoopEnd);
                        bufferingSource->getNextAudioBlock (firstPart);
                    }

                    bufferingSource->setNextReadPosition (loopStart);

                    juce::AudioSourceChannelInfo secondPart (info.buffer,
                                                              info.startSample + samplesUntilLoopEnd,
                                                              info.numSamples - samplesUntilLoopEnd);
                    bufferingSource->getNextAudioBlock (secondPart);
                    return;
                }
            }
        }

        bufferingSource->getNextAudioBlock (info);
    }

    void TrackAudioSource::setNextReadPosition (juce::int64 newPosition)
    {
        bufferingSource->setNextReadPosition (newPosition);
    }

    juce::int64 TrackAudioSource::getNextReadPosition() const
    {
        return bufferingSource->getNextReadPosition();
    }

    juce::int64 TrackAudioSource::getTotalLength() const
    {
        return bufferingSource->getTotalLength();
    }

    bool TrackAudioSource::isLooping() const
    {
        return loopEnabled.load (std::memory_order_acquire);
    }

    void TrackAudioSource::setLooping (bool shouldLoop)
    {
        loopEnabled.store (shouldLoop, std::memory_order_release);
    }

    void TrackAudioSource::setPlaying (bool shouldPlay)
    {
        playing.store (shouldPlay, std::memory_order_release);
    }

    bool TrackAudioSource::isPlaying() const noexcept
    {
        return playing.load (std::memory_order_acquire);
    }

    void TrackAudioSource::setLoopPointsSamples (juce::int64 startSample, juce::int64 endSample)
    {
        loopStartSample.store (startSample, std::memory_order_release);
        loopEndSample.store (endSample, std::memory_order_release);
    }
}

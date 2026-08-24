#include "WaveformView.h"

namespace vcdj::waveform
{
    WaveformView::WaveformView()
    {
        setInterceptsMouseClicks (true, false);
    }

    void WaveformView::setWaveform (WaveformData newData)
    {
        data = std::move (newData);
        repaint();
    }

    void WaveformView::setDeckSnapshot (const DeckSnapshot& snapshot)
    {
        lastSnapshot = snapshot;
        repaint();
    }

    int WaveformView::secondsToX (double seconds) const
    {
        if (lastSnapshot.durationSeconds <= 0.0)
            return 0;

        const double ratio = juce::jlimit (0.0, 1.0, seconds / lastSnapshot.durationSeconds);
        return (int) (ratio * (double) getWidth());
    }

    double WaveformView::xToSeconds (int x) const
    {
        if (getWidth() <= 0)
            return 0.0;

        const double ratio = juce::jlimit (0.0, 1.0, (double) x / (double) getWidth());
        return ratio * lastSnapshot.durationSeconds;
    }

    void WaveformView::paint (juce::Graphics& g)
    {
        auto bounds = getLocalBounds();
        g.fillAll (juce::Colour (0xff14181c));

        if (data.isEmpty())
        {
            g.setColour (juce::Colours::grey);
            g.drawText (lastSnapshot.trackLoaded ? "Analyzing waveform..." : "No track loaded",
                        bounds, juce::Justification::centred);
            return;
        }

        const int width = bounds.getWidth();
        const int height = bounds.getHeight();
        const float centerY = (float) height * 0.5f;
        const float halfHeight = (float) height * 0.48f;
        const auto numPeaks = (int) data.peaks.size();

        // Loop region shading, drawn under the waveform.
        if (lastSnapshot.hasLoopIn && lastSnapshot.hasLoopOut)
        {
            const int x1 = secondsToX (lastSnapshot.loopInSeconds);
            const int x2 = secondsToX (lastSnapshot.loopOutSeconds);
            g.setColour (lastSnapshot.loopEnabled ? juce::Colours::limegreen.withAlpha (0.18f)
                                                   : juce::Colours::yellow.withAlpha (0.10f));
            g.fillRect (juce::Rectangle<int> (x1, 0, juce::jmax (1, x2 - x1), height));
        }

        // Waveform peaks.
        g.setColour (juce::Colour (0xff34c3ff));

        for (int x = 0; x < width; ++x)
        {
            const int peakIndex = juce::jlimit (0, numPeaks - 1, (x * numPeaks) / juce::jmax (1, width));
            const auto& peak = data.peaks[(size_t) peakIndex];

            const float y1 = centerY - peak.maxValue * halfHeight;
            const float y2 = centerY - peak.minValue * halfHeight;
            g.drawVerticalLine (x, juce::jmin (y1, y2), juce::jmax (y1, y2, y1 + 1.0f));
        }

        // Hot cue markers.
        for (int i = 0; i < (int) lastSnapshot.hotCues.size(); ++i)
        {
            if (! lastSnapshot.hotCues[(size_t) i].has_value())
                continue;

            const int x = secondsToX (*lastSnapshot.hotCues[(size_t) i]);
            g.setColour (juce::Colours::orange);
            g.drawVerticalLine (x, 0.0f, (float) height);
            g.setFont (11.0f);
            g.drawText (juce::String (i + 1), x + 2, 2, 16, 14, juce::Justification::centredLeft);
        }

        // Cue marker.
        if (lastSnapshot.hasCue)
        {
            const int x = secondsToX (lastSnapshot.cuePositionSeconds);
            g.setColour (juce::Colours::red);
            g.drawVerticalLine (x, 0.0f, (float) height);
            g.drawText ("CUE", x + 2, height - 16, 40, 14, juce::Justification::centredLeft);
        }

        // Playhead.
        {
            const int x = secondsToX (lastSnapshot.positionSeconds);
            g.setColour (juce::Colours::white);
            g.drawVerticalLine (x, 0.0f, (float) height);
        }
    }

    void WaveformView::requestSeekFromMouse (const juce::MouseEvent& event)
    {
        if (onSeekRequested != nullptr)
            onSeekRequested (xToSeconds (event.x));
    }

    void WaveformView::mouseDown (const juce::MouseEvent& event) { requestSeekFromMouse (event); }
    void WaveformView::mouseDrag (const juce::MouseEvent& event) { requestSeekFromMouse (event); }
}

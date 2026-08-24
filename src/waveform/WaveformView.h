#pragma once

#include "WaveformData.h"
#include "../deck/DeckTypes.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

namespace vcdj::waveform
{
    /** Renders a cached WaveformData overview plus the playhead, cue marker,
        loop region and hot cue markers (ARCHITECTURE.md section 7.6).

        This component never touches the audio engine or DeckController
        directly - it only paints whatever it has been told, and reports
        click-to-seek requests via onSeekRequested so the owner (MainComponent)
        can route them through DeckController, keeping the dependency
        direction UI -> Controller -> Engine intact.
    */
    class WaveformView final : public juce::Component
    {
    public:
        WaveformView();

        void setWaveform (WaveformData newData);
        void setDeckSnapshot (const DeckSnapshot& snapshot);

        /** Called with a seconds position when the user clicks/drags on the
            waveform. */
        std::function<void (double)> onSeekRequested;

        void paint (juce::Graphics& g) override;
        void mouseDown (const juce::MouseEvent& event) override;
        void mouseDrag (const juce::MouseEvent& event) override;

    private:
        double xToSeconds (int x) const;
        int secondsToX (double seconds) const;
        void requestSeekFromMouse (const juce::MouseEvent& event);

        WaveformData data;
        DeckSnapshot lastSnapshot;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveformView)
    };
}

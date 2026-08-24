#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

namespace vcdj::ui
{
    /** The visual/interactive half of the virtual platter (ARCHITECTURE.md
        sections 12/13).

        PlatterView owns only presentation and raw mouse input; it knows
        nothing about the audio engine. Dragging the platter reports a
        relative time delta via onJog, and touching/releasing it reports
        onScratchActiveChanged - mirroring a vinyl-mode CDJ platter, where
        touching the platter pauses autoplay and releasing it resumes.
        MainComponent wires both callbacks into DeckController, which owns the
        actual ScratchEngine/audio-engine interaction.
    */
    class PlatterView final : public juce::Component
    {
    public:
        PlatterView();

        /** Updates the visual rotation to reflect current playback position. */
        void setRotation (double positionSeconds, double effectiveRatio);
        void setPlaying (bool isPlaying);
        void setScratchMode (bool isActive);

        std::function<void (double)> onJog;
        std::function<void (bool)> onScratchActiveChanged;

        void paint (juce::Graphics& g) override;
        void mouseDown (const juce::MouseEvent& event) override;
        void mouseDrag (const juce::MouseEvent& event) override;
        void mouseUp (const juce::MouseEvent& event) override;

    private:
        double angleAtPoint (juce::Point<float> p) const;

        double angleRadians = 0.0;
        bool playing = false;
        bool scratchMode = false;

        bool dragging = false;
        double dragStartAngle = 0.0;

        static constexpr double secondsPerRevolution = 1.8; // ~33 1/3 RPM

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlatterView)
    };
}

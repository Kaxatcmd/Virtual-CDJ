#pragma once

#include "PlatterView.h"
#include "../deck/DeckController.h"
#include "../audio/AudioEngine.h"
#include "../input/KeyboardInputManager.h"
#include "../waveform/WaveformView.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <array>

namespace vcdj::ui
{
    /** The whole application UI: a single compact CDJ-style deck
        (ARCHITECTURE.md section 7.7 / section 13 "UI").

        MainComponent owns the real AudioEngine and DeckController and is the
        composition root that wires UI widgets -> DeckController and
        DeckController state -> UI widgets. It never calls into AudioEngine
        directly.
    */
    class MainComponent final : public juce::Component,
                                 private juce::Timer
    {
    public:
        MainComponent();
        ~MainComponent() override;

        void paint (juce::Graphics& g) override;
        void resized() override;
        void visibilityChanged() override;

        bool keyPressed (const juce::KeyPress& key) override;

        /** Loads a track directly, bypassing the file chooser dialog. Used for
            the initial command-line file argument (and is handy for manual
            testing / OS "open with" integration). */
        void loadFile (const juce::File& file);

    private:
        void timerCallback() override;
        void chooseAndLoadFile();
        void refreshFromSnapshot();
        void updateHotCueButtons (const DeckSnapshot& snapshot);
        void updateLoopButtons (const DeckSnapshot& snapshot);

        static juce::String formatTime (double seconds);

        AudioEngine audioEngine;
        DeckController deckController { audioEngine };
        KeyboardInputManager inputManager { deckController };

        juce::TextButton loadButton { "Load Track..." };
        juce::Label trackNameLabel;
        juce::Label bpmLabel;
        juce::Label pitchLabel;
        juce::Label timeLabel;

        waveform::WaveformView waveformView;
        PlatterView platterView;

        juce::TextButton playPauseButton { "Play / Pause  [Space]" };
        juce::TextButton cueButton       { "Cue  [C]" };
        juce::TextButton returnCueButton { "Back to Cue  [Q]" };
        juce::TextButton stopButton      { "Stop  [Esc]" };

        juce::TextButton loopInButton    { "Loop In  [I]" };
        juce::TextButton loopOutButton   { "Loop Out  [O]" };
        juce::TextButton loopToggleButton{ "Loop On/Off  [L]" };

        juce::TextButton scratchButton   { "Scratch Mode  [S]" };

        std::array<juce::TextButton, HotCueStore::numSlots> hotCueButtons;

        juce::Label keyboardHintLabel;

        std::unique_ptr<juce::FileChooser> fileChooser;
        bool waveformFetched = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
    };
}

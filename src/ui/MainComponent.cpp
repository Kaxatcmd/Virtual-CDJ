#include "MainComponent.h"

namespace vcdj::ui
{
    namespace
    {
        constexpr int uiTimerHz = 30;

        juce::Colour panelBackground()  { return juce::Colour (0xff20242a); }
        juce::Colour appBackground()    { return juce::Colour (0xff101215); }
    }

    MainComponent::MainComponent()
    {
        setWantsKeyboardFocus (true);

        loadButton.onClick = [this] { chooseAndLoadFile(); };
        addAndMakeVisible (loadButton);

        for (auto* label : { &trackNameLabel, &bpmLabel, &pitchLabel, &timeLabel })
        {
            label->setColour (juce::Label::textColourId, juce::Colours::white);
            addAndMakeVisible (*label);
        }

        trackNameLabel.setText ("No track loaded", juce::dontSendNotification);
        trackNameLabel.setFont (juce::Font (juce::FontOptions (18.0f, juce::Font::bold)));
        bpmLabel.setText ("BPM: --", juce::dontSendNotification);
        pitchLabel.setText ("Pitch: +0.0%", juce::dontSendNotification);
        timeLabel.setText ("00:00.0 / 00:00.0", juce::dontSendNotification);
        timeLabel.setFont (juce::Font (juce::FontOptions (16.0f, juce::Font::bold)));

        addAndMakeVisible (waveformView);
        waveformView.onSeekRequested = [this] (double seconds) { deckController.seek (seconds); };

        addAndMakeVisible (platterView);
        platterView.onJog = [this] (double deltaSeconds) { deckController.jogPlatter (deltaSeconds); };
        platterView.onScratchActiveChanged = [this] (bool active) { deckController.setScratchActive (active); };

        playPauseButton.onClick  = [this] { deckController.togglePlayPause(); };
        cueButton.onClick        = [this] { deckController.setCue(); };
        returnCueButton.onClick  = [this] { deckController.returnToCue(); };
        stopButton.onClick       = [this] { deckController.stop(); };
        loopInButton.onClick     = [this] { deckController.loopIn(); };
        loopOutButton.onClick    = [this] { deckController.loopOut(); };
        loopToggleButton.onClick = [this] { deckController.toggleLoop(); };
        scratchButton.onClick    = [this] { deckController.toggleScratchMode(); };

        for (auto* button : { &playPauseButton, &cueButton, &returnCueButton, &stopButton,
                               &loopInButton, &loopOutButton, &loopToggleButton, &scratchButton })
        {
            // Keyboard shortcuts must keep working no matter what was last
            // clicked with the mouse, so buttons must not steal keyboard focus
            // away from this component (ARCHITECTURE.md section 4: keyboard
            // control is first-class, not incidental).
            button->setMouseClickGrabsKeyboardFocus (false);
            addAndMakeVisible (*button);
        }

        for (int i = 0; i < HotCueStore::numSlots; ++i)
        {
            auto& button = hotCueButtons[(size_t) i];
            button.setButtonText (juce::String (i + 1));
            button.onClick = [this, i] { deckController.triggerHotCue (i); };
            button.setMouseClickGrabsKeyboardFocus (false);
            addAndMakeVisible (button);
        }

        loadButton.setMouseClickGrabsKeyboardFocus (false);

        keyboardHintLabel.setJustificationType (juce::Justification::topLeft);
        keyboardHintLabel.setColour (juce::Label::textColourId, juce::Colours::lightgrey);
        keyboardHintLabel.setFont (juce::Font (juce::FontOptions (13.0f)));
        keyboardHintLabel.setText (
            "Space Play/Pause   C Cue   Q Back-to-Cue   Esc Stop\n"
            "I Loop In   O Loop Out   L Loop On/Off\n"
            "Up/Down Pitch +/-0.1%   R Reset Pitch   Left/Right Temporary Bend\n"
            "1-8 Hot Cues   S Scratch Mode   A/D Platter Jog",
            juce::dontSendNotification);
        addAndMakeVisible (keyboardHintLabel);

        setSize (960, 640);
        startTimerHz (uiTimerHz);
    }

    MainComponent::~MainComponent()
    {
        stopTimer();
    }

    void MainComponent::paint (juce::Graphics& g)
    {
        g.fillAll (appBackground());

        auto panel = getLocalBounds().reduced (8).removeFromTop (170);
        g.setColour (panelBackground());
        g.fillRoundedRectangle (panel.toFloat(), 6.0f);
    }

    void MainComponent::resized()
    {
        auto area = getLocalBounds().reduced (8);

        // --- Top info panel ---
        auto topPanel = area.removeFromTop (170).reduced (10);

        auto headerRow = topPanel.removeFromTop (28);
        loadButton.setBounds (headerRow.removeFromLeft (140));
        headerRow.removeFromLeft (10);
        trackNameLabel.setBounds (headerRow);

        topPanel.removeFromTop (6);
        auto statsRow = topPanel.removeFromTop (24);
        bpmLabel.setBounds (statsRow.removeFromLeft (140));
        pitchLabel.setBounds (statsRow.removeFromLeft (160));
        timeLabel.setBounds (statsRow);

        topPanel.removeFromTop (10);
        auto transportRow = topPanel.removeFromTop (32);
        const int transportButtonWidth = 150;
        for (auto* button : { &playPauseButton, &cueButton, &returnCueButton, &stopButton })
        {
            button->setBounds (transportRow.removeFromLeft (transportButtonWidth));
            transportRow.removeFromLeft (6);
        }

        topPanel.removeFromTop (8);
        auto loopRow = topPanel.removeFromTop (32);
        for (auto* button : { &loopInButton, &loopOutButton, &loopToggleButton, &scratchButton })
        {
            button->setBounds (loopRow.removeFromLeft (transportButtonWidth));
            loopRow.removeFromLeft (6);
        }

        area.removeFromTop (10);

        // --- Waveform ---
        waveformView.setBounds (area.removeFromTop (140));

        area.removeFromTop (10);

        // --- Hot cues ---
        auto hotCueRow = area.removeFromTop (36);
        const int hotCueWidth = hotCueRow.getWidth() / HotCueStore::numSlots;
        for (auto& button : hotCueButtons)
        {
            button.setBounds (hotCueRow.removeFromLeft (hotCueWidth).reduced (3));
        }

        area.removeFromTop (10);

        // --- Bottom: platter + keyboard hints ---
        auto bottomArea = area;
        platterView.setBounds (bottomArea.removeFromLeft (bottomArea.getHeight()).reduced (4));
        bottomArea.removeFromLeft (12);
        keyboardHintLabel.setBounds (bottomArea);
    }

    bool MainComponent::keyPressed (const juce::KeyPress& key)
    {
        return inputManager.handleKeyPress (key);
    }

    void MainComponent::visibilityChanged()
    {
        // Keyboard shortcuts are the primary control surface for this app
        // (ARCHITECTURE.md section 4), so make sure this component - not
        // whichever child button JUCE would otherwise pick - owns keyboard
        // focus as soon as the window becomes visible.
        if (isVisible())
            grabKeyboardFocus();
    }

    void MainComponent::chooseAndLoadFile()
    {
        fileChooser = std::make_unique<juce::FileChooser> (
            "Select an audio file to load",
            juce::File::getSpecialLocation (juce::File::userMusicDirectory),
            "*.wav;*.flac;*.mp3;*.aiff;*.aif;*.ogg");

        const auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

        fileChooser->launchAsync (chooserFlags, [this] (const juce::FileChooser& chooser)
        {
            const auto file = chooser.getResult();

            if (! file.existsAsFile())
                return;

            loadFile (file);
        });
    }

    void MainComponent::loadFile (const juce::File& file)
    {
        if (! file.existsAsFile())
            return;

        auto result = deckController.loadTrack (file);

        if (! result.success)
        {
            juce::AlertWindow::showAsync (
                juce::MessageBoxOptions()
                    .withIconType (juce::MessageBoxIconType::WarningIcon)
                    .withTitle ("Could not load track")
                    .withMessage (result.errorMessage)
                    .withButton ("OK"),
                nullptr);
        }

        waveformFetched = false;
        waveformView.setWaveform ({});
    }

    juce::String MainComponent::formatTime (double seconds)
    {
        if (seconds < 0.0 || std::isnan (seconds))
            seconds = 0.0;

        const int minutes = (int) seconds / 60;
        const double remainder = seconds - (double) (minutes * 60);
        return juce::String::formatted ("%02d:%04.1f", minutes, remainder);
    }

    void MainComponent::updateLoopButtons (const DeckSnapshot& snapshot)
    {
        loopToggleButton.setColour (juce::TextButton::buttonColourId,
                                     snapshot.loopEnabled ? juce::Colours::limegreen.withAlpha (0.6f)
                                                           : juce::Colours::darkgrey);
    }

    void MainComponent::updateHotCueButtons (const DeckSnapshot& snapshot)
    {
        for (int i = 0; i < HotCueStore::numSlots; ++i)
        {
            const bool isSet = snapshot.hotCues[(size_t) i].has_value();
            hotCueButtons[(size_t) i].setColour (juce::TextButton::buttonColourId,
                                                  isSet ? juce::Colours::orange.withAlpha (0.7f)
                                                        : juce::Colours::darkgrey);
        }
    }

    void MainComponent::refreshFromSnapshot()
    {
        const auto snapshot = deckController.getSnapshot();

        trackNameLabel.setText (snapshot.trackLoaded ? deckController.getTrackName() : juce::String ("No track loaded"),
                                 juce::dontSendNotification);

        bpmLabel.setText (! snapshot.trackLoaded
                               ? "BPM: --"
                               : (snapshot.bpmAnalysisComplete
                                      ? (snapshot.bpm > 0.0 ? "BPM: " + juce::String (snapshot.bpm, 1) : "BPM: n/a")
                                      : "BPM: analyzing..."),
                           juce::dontSendNotification);

        pitchLabel.setText (juce::String::formatted ("Pitch: %+.1f%%", snapshot.basePitchPercent + snapshot.bendPercent),
                             juce::dontSendNotification);

        timeLabel.setText (formatTime (snapshot.positionSeconds) + " / " + formatTime (snapshot.durationSeconds),
                            juce::dontSendNotification);

        playPauseButton.setButtonText (snapshot.playing ? "Pause  [Space]" : "Play  [Space]");
        scratchButton.setColour (juce::TextButton::buttonColourId,
                                  snapshot.scratchMode ? juce::Colours::orangered.withAlpha (0.7f)
                                                        : juce::Colours::darkgrey);

        updateLoopButtons (snapshot);
        updateHotCueButtons (snapshot);

        waveformView.setDeckSnapshot (snapshot);
        platterView.setRotation (snapshot.positionSeconds, snapshot.effectiveRatio);
        platterView.setPlaying (snapshot.playing);
        platterView.setScratchMode (snapshot.scratchMode);

        if (snapshot.bpmAnalysisComplete && ! waveformFetched)
        {
            auto waveform = deckController.getWaveform();

            if (! waveform.isEmpty())
            {
                waveformView.setWaveform (std::move (waveform));
                waveformFetched = true;
            }
        }
    }

    void MainComponent::timerCallback()
    {
        deckController.tick (1.0 / (double) uiTimerHz);
        refreshFromSnapshot();
    }
}

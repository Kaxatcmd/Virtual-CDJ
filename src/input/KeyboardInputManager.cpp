#include "KeyboardInputManager.h"

namespace vcdj
{
    namespace
    {
        // How far the virtual platter jogs the playback position per polling
        // tick while A/D is held. Kept modest and documented as a "search
        // jog" rather than a physically accurate scratch (ARCHITECTURE.md
        // section 12/13 - full scratch DSP is out of scope for this pass).
        constexpr double jogSecondsPerTick = 0.02;
        constexpr int pollingHz = 100;
    }

    KeyboardInputManager::KeyboardInputManager (DeckController& controllerIn)
        : controller (controllerIn)
    {
        addDefaultMappings();
        startTimerHz (pollingHz);
    }

    KeyboardInputManager::~KeyboardInputManager()
    {
        stopTimer();
    }

    void KeyboardInputManager::addDefaultMappings()
    {
        using K = juce::KeyPress;

        discreteMap = {
            { K (K::spaceKey),   CommandId::playPause },
            { K (K::escapeKey),  CommandId::stop },
            { K ((int) 'C'),     CommandId::cue },
            { K ((int) 'Q'),     CommandId::returnToCue },
            { K ((int) 'I'),     CommandId::loopIn },
            { K ((int) 'O'),     CommandId::loopOut },
            { K ((int) 'L'),     CommandId::loopToggle },
            { K ((int) 'R'),     CommandId::pitchReset },
            { K (K::upKey),      CommandId::pitchNudgeUp },
            { K (K::downKey),    CommandId::pitchNudgeDown },
            { K ((int) 'S'),     CommandId::scratchToggle },
            { K ((int) '1'),     CommandId::hotCue1 },
            { K ((int) '2'),     CommandId::hotCue2 },
            { K ((int) '3'),     CommandId::hotCue3 },
            { K ((int) '4'),     CommandId::hotCue4 },
            { K ((int) '5'),     CommandId::hotCue5 },
            { K ((int) '6'),     CommandId::hotCue6 },
            { K ((int) '7'),     CommandId::hotCue7 },
            { K ((int) '8'),     CommandId::hotCue8 },
        };
    }

    void KeyboardInputManager::remap (const juce::KeyPress& key, CommandId command)
    {
        for (auto& entry : discreteMap)
        {
            if (entry.second == command)
            {
                entry.first = key;
                return;
            }
        }

        discreteMap.emplace_back (key, command);
    }

    bool KeyboardInputManager::handleKeyPress (const juce::KeyPress& key)
    {
        for (auto& entry : discreteMap)
        {
            if (entry.first == key)
            {
                controller.execute (entry.second);
                return true;
            }
        }

        return false;
    }

    void KeyboardInputManager::timerCallback()
    {
        // Left/Right arrows: temporary pitch bend, with explicit
        // pressed/held/released edges (ARCHITECTURE.md section 5).
        const bool leftDown  = juce::KeyPress::isKeyCurrentlyDown (juce::KeyPress::leftKey);
        const bool rightDown = juce::KeyPress::isKeyCurrentlyDown (juce::KeyPress::rightKey);

        if (leftDown && ! leftArrowWasDown)
            controller.beginPitchBend (false);
        else if (! leftDown && leftArrowWasDown)
            controller.endPitchBend();

        if (rightDown && ! rightArrowWasDown)
            controller.beginPitchBend (true);
        else if (! rightDown && rightArrowWasDown)
            controller.endPitchBend();

        leftArrowWasDown = leftDown;
        rightArrowWasDown = rightDown;

        // A/D: virtual platter jog, active only while held.
        const bool aDown = juce::KeyPress::isKeyCurrentlyDown ((int) 'A');
        const bool dDown = juce::KeyPress::isKeyCurrentlyDown ((int) 'D');

        if (aDown)
            controller.jogPlatter (-jogSecondsPerTick);

        if (dDown)
            controller.jogPlatter (jogSecondsPerTick);
    }
}

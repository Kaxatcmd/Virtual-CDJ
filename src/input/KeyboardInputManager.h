#pragma once

#include "Commands.h"
#include "../deck/DeckController.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>
#include <utility>

namespace vcdj
{
    /** Translates physical keyboard events into DeckController calls
        (ARCHITECTURE.md sections 4, 5 and 7.3).

        This is the *only* place in the whole application that knows about
        actual key codes. Everything else deals in CommandId values or direct,
        named DeckController methods.

        Two input styles are handled, matching the "pressed / held / released"
        requirement in ARCHITECTURE.md section 5:

          - Discrete keys (Space, C, Q, I, O, L, R, 1-8, S, Esc) fire once per
            key-down via handleKeyPress(), routed through the CommandId table
            below so the mapping can be changed without touching this class's
            logic.

          - Continuous keys (Left/Right for pitch bend, A/D for the virtual
            platter jog) are polled on a timer so we can distinguish the
            press/hold/release edges ourselves and drive DeckController's
            begin/end-style APIs, which a single "key down" event cannot
            express.
    */
    class KeyboardInputManager final : private juce::Timer
    {
    public:
        explicit KeyboardInputManager (DeckController& controllerIn);
        ~KeyboardInputManager() override;

        /** Call from Component::keyPressed. Returns true if the key was
            recognised as a discrete command. */
        bool handleKeyPress (const juce::KeyPress& key);

        /** Replaces the mapping for a discrete command, so the keyboard layout
            can be reconfigured without recompiling DeckController or the UI. */
        void remap (const juce::KeyPress& key, CommandId command);

    private:
        void timerCallback() override;
        void addDefaultMappings();

        DeckController& controller;
        std::vector<std::pair<juce::KeyPress, CommandId>> discreteMap;

        bool leftArrowWasDown = false;
        bool rightArrowWasDown = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeyboardInputManager)
    };
}

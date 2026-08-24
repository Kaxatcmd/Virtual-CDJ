#include "deck/DeckController.h"
#include "input/KeyboardInputManager.h"
#include "FakeAudioEngine.h"
#include <juce_core/juce_core.h>

namespace
{
    class CommandMappingTests final : public juce::UnitTest
    {
    public:
        CommandMappingTests() : juce::UnitTest ("KeyboardInputManager mapping", "Input") {}

        void runTest() override
        {
            FakeAudioEngine engine;
            engine.loadFile (juce::File());
            engine.setDurationForTest (60.0);
            vcdj::DeckController controller (engine);
            vcdj::KeyboardInputManager input (controller);

            beginTest ("Space toggles play/pause");
            {
                expect (! controller.isPlaying());
                expect (input.handleKeyPress (juce::KeyPress (juce::KeyPress::spaceKey)));
                expect (controller.isPlaying());
                expect (input.handleKeyPress (juce::KeyPress (juce::KeyPress::spaceKey)));
                expect (! controller.isPlaying());
            }

            beginTest ("C sets a cue at the current position");
            {
                controller.seek (3.0);
                expect (input.handleKeyPress (juce::KeyPress ((int) 'C')));
                expect (controller.getCueState().hasCue());
                expectWithinAbsoluteError (controller.getCueState().getPosition(), 3.0, 0.01);
            }

            beginTest ("Q returns to the cue point");
            {
                controller.seek (40.0);
                expect (input.handleKeyPress (juce::KeyPress ((int) 'Q')));
                expectWithinAbsoluteError (controller.getPosition(), 3.0, 0.01);
            }

            beginTest ("Digit keys 1-8 trigger the matching hot cue");
            {
                controller.seek (7.0);
                expect (input.handleKeyPress (juce::KeyPress ((int) '1')));
                expect (controller.getHotCueStore().isSet (0));

                controller.seek (9.0);
                expect (input.handleKeyPress (juce::KeyPress ((int) '8')));
                expect (controller.getHotCueStore().isSet (7));
            }

            beginTest ("R resets pitch");
            {
                controller.nudgePitch (5.0);
                expect (input.handleKeyPress (juce::KeyPress ((int) 'R')));
                expectWithinAbsoluteError (controller.getPitchEngine().getBasePitchPercent(), 0.0, 0.0001);
            }

            beginTest ("Esc stops playback");
            {
                controller.play();
                expect (input.handleKeyPress (juce::KeyPress (juce::KeyPress::escapeKey)));
                expect (! controller.isPlaying());
            }

            beginTest ("An unmapped key is reported as unhandled");
            {
                expect (! input.handleKeyPress (juce::KeyPress ((int) 'Z')));
            }
        }
    };

    CommandMappingTests commandMappingTests;
}

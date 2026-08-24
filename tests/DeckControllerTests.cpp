#include "deck/DeckController.h"
#include "FakeAudioEngine.h"
#include <juce_core/juce_core.h>

namespace
{
    class DeckControllerTests final : public juce::UnitTest
    {
    public:
        DeckControllerTests() : juce::UnitTest ("DeckController", "Deck") {}

        void runTest() override
        {
            beginTest ("clampSeekPosition clamps into [0, duration]");
            {
                expectWithinAbsoluteError (vcdj::DeckController::clampSeekPosition (-5.0, 100.0), 0.0, 0.0001);
                expectWithinAbsoluteError (vcdj::DeckController::clampSeekPosition (150.0, 100.0), 100.0, 0.0001);
                expectWithinAbsoluteError (vcdj::DeckController::clampSeekPosition (50.0, 100.0), 50.0, 0.0001);
                expectWithinAbsoluteError (vcdj::DeckController::clampSeekPosition (5.0, 0.0), 0.0, 0.0001);
            }

            beginTest ("Play/pause/stop state transitions");
            {
                FakeAudioEngine engine;
                engine.loadFile (juce::File());
                vcdj::DeckController controller (engine);

                expect (! controller.isPlaying());
                controller.play();
                expect (controller.isPlaying());
                controller.pause();
                expect (! controller.isPlaying());

                controller.play();
                controller.stop();
                expect (! controller.isPlaying());
                expectWithinAbsoluteError (controller.getPosition(), 0.0, 0.0001);
            }

            beginTest ("Seek clamps through DeckController");
            {
                FakeAudioEngine engine;
                engine.loadFile (juce::File());
                engine.setDurationForTest (60.0);
                vcdj::DeckController controller (engine);

                controller.seek (1000.0);
                expectWithinAbsoluteError (controller.getPosition(), 60.0, 0.0001);

                controller.seek (-10.0);
                expectWithinAbsoluteError (controller.getPosition(), 0.0, 0.0001);
            }

            beginTest ("Return-to-cue creates a cue at 0 if none was set");
            {
                FakeAudioEngine engine;
                engine.loadFile (juce::File());
                engine.setDurationForTest (60.0);
                vcdj::DeckController controller (engine);

                controller.seek (30.0);
                controller.returnToCue();

                expectWithinAbsoluteError (controller.getPosition(), 0.0, 0.0001);
                expect (controller.getCueState().hasCue());
            }

            beginTest ("Return-to-cue recalls a previously set cue");
            {
                FakeAudioEngine engine;
                engine.loadFile (juce::File());
                engine.setDurationForTest (60.0);
                vcdj::DeckController controller (engine);

                controller.seek (15.0);
                controller.setCue();
                controller.seek (45.0);
                controller.returnToCue();

                expectWithinAbsoluteError (controller.getPosition(), 15.0, 0.0001);
            }

            beginTest ("Loop only enables once both in and out points are valid");
            {
                FakeAudioEngine engine;
                engine.loadFile (juce::File());
                engine.setDurationForTest (60.0);
                vcdj::DeckController controller (engine);

                controller.seek (5.0);
                controller.loopIn();
                controller.toggleLoop();
                expect (! controller.getLoopState().isEnabled());

                controller.seek (10.0);
                controller.loopOut();
                controller.toggleLoop();
                expect (controller.getLoopState().isEnabled());
            }

            beginTest ("Hot cue trigger jumps to a previously stored slot");
            {
                FakeAudioEngine engine;
                engine.loadFile (juce::File());
                engine.setDurationForTest (60.0);
                vcdj::DeckController controller (engine);

                controller.seek (20.0);
                controller.triggerHotCue (0);
                controller.seek (50.0);
                controller.triggerHotCue (0);

                expectWithinAbsoluteError (controller.getPosition(), 20.0, 0.0001);
            }

            beginTest ("Loading a new track resets cue/loop/hot cues/pitch");
            {
                FakeAudioEngine engine;
                engine.setDurationForTest (60.0);
                vcdj::DeckController controller (engine);

                controller.seek (10.0);
                controller.setCue();
                controller.nudgePitch (3.0);

                controller.loadTrack (juce::File());

                expect (! controller.getCueState().hasCue());
                expect (! controller.getLoopState().isEnabled());
                expectWithinAbsoluteError (controller.getPitchEngine().getBasePitchPercent(), 0.0, 0.0001);
            }
        }
    };

    DeckControllerTests deckControllerTests;
}

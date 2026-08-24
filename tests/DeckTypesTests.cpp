#include "deck/DeckTypes.h"
#include <juce_core/juce_core.h>

namespace
{
    class CueStateTests final : public juce::UnitTest
    {
    public:
        CueStateTests() : juce::UnitTest ("CueState", "Deck") {}

        void runTest() override
        {
            beginTest ("No cue is set initially");
            {
                vcdj::CueState cue;
                expect (! cue.hasCue());
            }

            beginTest ("Setting a cue stores its position");
            {
                vcdj::CueState cue;
                cue.setCue (12.5);
                expect (cue.hasCue());
                expectWithinAbsoluteError (cue.getPosition(), 12.5, 0.0001);
            }

            beginTest ("Clearing removes the cue");
            {
                vcdj::CueState cue;
                cue.setCue (5.0);
                cue.clear();
                expect (! cue.hasCue());
            }
        }
    };

    CueStateTests cueStateTests;

    class LoopStateTests final : public juce::UnitTest
    {
    public:
        LoopStateTests() : juce::UnitTest ("LoopState", "Deck") {}

        void runTest() override
        {
            beginTest ("Invalid until both points are set with sufficient length");
            {
                vcdj::LoopState loop;
                expect (! loop.isValid());

                loop.setLoopIn (10.0);
                expect (! loop.isValid());

                loop.setLoopOut (10.0 + vcdj::LoopState::minimumLengthSeconds * 0.5);
                expect (! loop.isValid());

                loop.setLoopOut (10.0 + vcdj::LoopState::minimumLengthSeconds * 2.0);
                expect (loop.isValid());
            }

            beginTest ("Enabling only sticks when the loop is valid");
            {
                vcdj::LoopState loop;
                loop.setEnabled (true);
                expect (! loop.isEnabled());

                loop.setLoopIn (1.0);
                loop.setLoopOut (2.0);
                loop.setEnabled (true);
                expect (loop.isEnabled());
            }

            beginTest ("Toggle flips the enabled state when valid");
            {
                vcdj::LoopState loop;
                loop.setLoopIn (0.0);
                loop.setLoopOut (1.0);

                loop.toggle();
                expect (loop.isEnabled());

                loop.toggle();
                expect (! loop.isEnabled());
            }

            beginTest ("Clear resets in/out/enabled");
            {
                vcdj::LoopState loop;
                loop.setLoopIn (0.0);
                loop.setLoopOut (1.0);
                loop.setEnabled (true);

                loop.clear();
                expect (! loop.hasLoopIn());
                expect (! loop.hasLoopOut());
                expect (! loop.isEnabled());
            }
        }
    };

    LoopStateTests loopStateTests;

    class HotCueStoreTests final : public juce::UnitTest
    {
    public:
        HotCueStoreTests() : juce::UnitTest ("HotCueStore", "Deck") {}

        void runTest() override
        {
            beginTest ("Triggering an empty slot stores the position");
            {
                vcdj::HotCueStore store;
                auto result = store.trigger (0, 42.0);
                expect (result.action == vcdj::HotCueStore::TriggerAction::setAt);
                expectWithinAbsoluteError (result.positionSeconds, 42.0, 0.0001);
                expect (store.isSet (0));
            }

            beginTest ("Triggering a set slot jumps without overwriting it");
            {
                vcdj::HotCueStore store;
                store.trigger (2, 10.0);
                auto result = store.trigger (2, 99.0);
                expect (result.action == vcdj::HotCueStore::TriggerAction::jumpTo);
                expectWithinAbsoluteError (result.positionSeconds, 10.0, 0.0001);
            }

            beginTest ("Clearing a slot empties it");
            {
                vcdj::HotCueStore store;
                store.trigger (3, 5.0);
                store.clear (3);
                expect (! store.isSet (3));
            }

            beginTest ("All 8 slots are independent");
            {
                vcdj::HotCueStore store;
                for (int i = 0; i < vcdj::HotCueStore::numSlots; ++i)
                    store.trigger (i, (double) i * 10.0);

                for (int i = 0; i < vcdj::HotCueStore::numSlots; ++i)
                    expect (store.isSet (i));
            }
        }
    };

    HotCueStoreTests hotCueStoreTests;
}

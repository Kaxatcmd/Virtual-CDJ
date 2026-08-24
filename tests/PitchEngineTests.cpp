#include "deck/PitchEngine.h"
#include <juce_core/juce_core.h>

namespace
{
    class PitchEngineTests final : public juce::UnitTest
    {
    public:
        PitchEngineTests() : juce::UnitTest ("PitchEngine", "Deck") {}

        void runTest() override
        {
            beginTest ("Base pitch clamps to the configured range");
            {
                vcdj::PitchEngine engine (8.0, 6.0);
                engine.setBasePitchPercent (100.0);
                expectWithinAbsoluteError (engine.getBasePitchPercent(), 8.0, 0.0001);

                engine.setBasePitchPercent (-100.0);
                expectWithinAbsoluteError (engine.getBasePitchPercent(), -8.0, 0.0001);
            }

            beginTest ("Reset returns base pitch to zero");
            {
                vcdj::PitchEngine engine;
                engine.setBasePitchPercent (4.0);
                engine.resetPitch();
                expectWithinAbsoluteError (engine.getBasePitchPercent(), 0.0, 0.0001);
            }

            beginTest ("Holding a bend approaches the configured bend amount");
            {
                vcdj::PitchEngine engine (8.0, 6.0);
                engine.setBasePitchPercent (2.0);
                engine.beginBend (true);
                engine.tick (10.0); // far more time than needed to fully settle
                expectWithinAbsoluteError (engine.getBendPercent(), 6.0, 0.01);
                expectWithinAbsoluteError (engine.getEffectiveRatio(), 1.0 + (2.0 + 6.0) / 100.0, 0.0001);
            }

            beginTest ("Releasing a bend smoothly returns it to zero");
            {
                vcdj::PitchEngine engine;
                engine.beginBend (true);
                engine.tick (10.0);
                engine.endBend();
                engine.tick (10.0);
                expectWithinAbsoluteError (engine.getBendPercent(), 0.0, 0.01);
            }

            beginTest ("Effective ratio of 1.0 when pitch is neutral");
            {
                vcdj::PitchEngine engine;
                expectWithinAbsoluteError (engine.getEffectiveRatio(), 1.0, 0.0001);
            }
        }
    };

    PitchEngineTests pitchEngineTests;
}

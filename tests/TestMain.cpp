#include <juce_core/juce_core.h>
#include <cstdio>

/** Runs every registered juce::UnitTest (see the *.cpp files in this
    directory) and returns a non-zero exit code if anything failed, so this
    can be wired into ctest / CI. Kept deliberately free of any JUCE
    application/message-loop machinery - these are all fast, deterministic
    domain-logic tests (ARCHITECTURE.md section 20).
*/
int main (int, char**)
{
    juce::UnitTestRunner runner;
    runner.setAssertOnFailure (false);
    runner.runAllTests();

    int totalFailures = 0;

    for (int i = 0; i < runner.getNumResults(); ++i)
    {
        auto* result = runner.getResult (i);

        if (result != nullptr && result->failures > 0)
        {
            std::printf ("FAILED: %s / %s (%d failures)\n",
                          result->unitTestName.toRawUTF8(),
                          result->subcategoryName.toRawUTF8(),
                          result->failures);
            totalFailures += result->failures;
        }
    }

    if (totalFailures == 0)
        std::printf ("All unit tests passed.\n");

    return totalFailures == 0 ? 0 : 1;
}

#include "AppPaths.h"

namespace vcdj
{
    juce::File AppPaths::getAnalysisCacheDirectory()
    {
        auto dir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                       .getChildFile ("VirtualCDJ")
                       .getChildFile ("analysis-cache");

        if (! dir.isDirectory())
            dir.createDirectory();

        return dir;
    }
}

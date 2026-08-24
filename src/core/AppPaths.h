#pragma once

#include <juce_core/juce_core.h>

namespace vcdj
{
    /** Cross-cutting filesystem helpers.

        Analysis results (waveform peaks + BPM estimate) are cached outside of the
        user's music library so that we never write next to, or modify, the original
        audio files (see ARCHITECTURE.md section 15, "Persistence").
    */
    namespace AppPaths
    {
        /** Returns (and creates if necessary) the directory used to cache per-track
            analysis results. */
        juce::File getAnalysisCacheDirectory();
    }
}

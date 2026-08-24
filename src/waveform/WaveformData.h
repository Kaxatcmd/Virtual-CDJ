#pragma once

#include <vector>

namespace vcdj::waveform
{
    /** One column of a min/max peak-envelope waveform display. */
    struct Peak
    {
        float minValue = 0.0f;
        float maxValue = 0.0f;
    };

    /** Cached/generated waveform overview data (ARCHITECTURE.md section 7.6).

        Generated asynchronously by analysis::TrackAnalyzer - never touched
        from the real-time audio callback. The resolution (number of peaks) is
        fixed regardless of track length so the UI can draw it directly without
        any further resampling.
    */
    struct WaveformData
    {
        std::vector<Peak> peaks;
        double durationSeconds = 0.0;

        bool isEmpty() const noexcept { return peaks.empty(); }
    };
}

#pragma once

#include <atomic>
#include <algorithm>

namespace vcdj
{
    /** Implements the pitch model from ARCHITECTURE.md section 8:

            effective_pitch = base_pitch + pitch_bend
            effective_ratio = 1.0 + effective_pitch / 100.0

        base_pitch is the "permanent" fader-style pitch set by the user (Up/Down
        arrows, reset with R). pitch_bend is the temporary offset applied while
        a bend key is held (Left/Right arrows) and smoothly decays back to zero
        on release rather than snapping, so it feels like nudging a platter
        rather than flipping a switch.

        All state is atomic so PitchEngine can be written from the input/UI
        thread and read from wherever the effective ratio is pushed into the
        audio engine (also the UI/controller thread in this project - there is
        no audio-thread access to this class at all).
    */
    class PitchEngine
    {
    public:
        explicit PitchEngine (double pitchRangePercentIn = 8.0, double bendAmountPercentIn = 6.0)
            : pitchRangePercent (pitchRangePercentIn), bendAmountPercent (bendAmountPercentIn)
        {
        }

        void setPitchRangePercent (double range) { pitchRangePercent = std::abs (range); }
        double getPitchRangePercent() const { return pitchRangePercent; }

        void setBendAmountPercent (double amount) { bendAmountPercent = std::abs (amount); }

        /** Sets the permanent base pitch, clamped to +/- pitchRangePercent. */
        void setBasePitchPercent (double percent)
        {
            basePitchPercent.store (clampToRange (percent), std::memory_order_release);
        }

        double getBasePitchPercent() const { return basePitchPercent.load (std::memory_order_acquire); }

        /** Nudges the base pitch by a small step (Up/Down arrows), clamped. */
        void nudgeBasePitch (double deltaPercent)
        {
            setBasePitchPercent (getBasePitchPercent() + deltaPercent);
        }

        void resetPitch()
        {
            basePitchPercent.store (0.0, std::memory_order_release);
        }

        /** Left/Right arrow pressed: begin bending towards +/- bendAmountPercent. */
        void beginBend (bool positiveDirection)
        {
            targetBendPercent.store (positiveDirection ? bendAmountPercent : -bendAmountPercent,
                                      std::memory_order_release);
        }

        /** Left/Right arrow released: smoothly return to zero bend. */
        void endBend()
        {
            targetBendPercent.store (0.0, std::memory_order_release);
        }

        /** Advances the bend smoothing. Call periodically (e.g. from a ~50Hz UI
            timer) - this is cheap, non-blocking arithmetic, never audio-callback
            code. */
        void tick (double deltaSeconds)
        {
            const double target = targetBendPercent.load (std::memory_order_acquire);
            const double current = currentBendPercent.load (std::memory_order_acquire);

            if (current == target)
                return;

            const double maxStep = bendSmoothingRatePerSecond * deltaSeconds;
            const double diff = target - current;
            const double step = std::clamp (diff, -maxStep, maxStep);

            currentBendPercent.store (current + step, std::memory_order_release);
        }

        double getBendPercent() const { return currentBendPercent.load (std::memory_order_acquire); }

        /** effective_ratio = 1.0 + (base_pitch + bend) / 100 */
        double getEffectiveRatio() const
        {
            const double effectivePercent = getBasePitchPercent() + getBendPercent();
            return 1.0 + effectivePercent / 100.0;
        }

    private:
        double clampToRange (double percent) const
        {
            return std::clamp (percent, -pitchRangePercent, pitchRangePercent);
        }

        double pitchRangePercent;
        double bendAmountPercent;
        double bendSmoothingRatePerSecond = 40.0; // percent per second approach rate

        std::atomic<double> basePitchPercent { 0.0 };
        std::atomic<double> currentBendPercent { 0.0 };
        std::atomic<double> targetBendPercent { 0.0 };
    };
}

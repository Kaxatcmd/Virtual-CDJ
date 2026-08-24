#pragma once

#include <array>
#include <atomic>
#include <optional>
#include <juce_core/juce_core.h>

namespace vcdj
{
    /** Primary cue point (ARCHITECTURE.md section 9).

        Chosen semantics (documented here since the architecture brief leaves the
        exact behaviour open, section 9 / section 6 of the README):

            - Pressing Cue sets the cue point at the current position.
            - Pressing Return-to-Cue seeks back to the cue point (creating one
              at position 0 first if none has been set yet), leaving the
              current play/pause state unchanged - a simple, predictable
              "recall position" rather than a full preview/hold workflow.

        This is intentionally simple and predictable rather than trying to
        emulate every nuance of a specific commercial CDJ.
    */
    class CueState
    {
    public:
        void setCue (double positionSeconds)
        {
            cuePositionSeconds.store (positionSeconds, std::memory_order_release);
            cueIsSet.store (true, std::memory_order_release);
        }

        void clear()
        {
            cueIsSet.store (false, std::memory_order_release);
            cuePositionSeconds.store (0.0, std::memory_order_release);
        }

        bool hasCue() const { return cueIsSet.load (std::memory_order_acquire); }
        double getPosition() const { return cuePositionSeconds.load (std::memory_order_acquire); }

    private:
        std::atomic<bool> cueIsSet { false };
        std::atomic<double> cuePositionSeconds { 0.0 };
    };

    /** Manual (non beat-quantized) loop state (ARCHITECTURE.md section 10). */
    class LoopState
    {
    public:
        static constexpr double minimumLengthSeconds = 0.05;

        void setLoopIn (double positionSeconds)
        {
            loopInSeconds.store (positionSeconds, std::memory_order_release);
            hasIn.store (true, std::memory_order_release);
        }

        void setLoopOut (double positionSeconds)
        {
            loopOutSeconds.store (positionSeconds, std::memory_order_release);
            hasOut.store (true, std::memory_order_release);
        }

        bool hasLoopIn() const { return hasIn.load (std::memory_order_acquire); }
        bool hasLoopOut() const { return hasOut.load (std::memory_order_acquire); }
        double getLoopIn() const { return loopInSeconds.load (std::memory_order_acquire); }
        double getLoopOut() const { return loopOutSeconds.load (std::memory_order_acquire); }

        /** A loop is only valid once both points are set, in the right order,
            and long enough to be audible/loopable without glitching. */
        bool isValid() const
        {
            return hasLoopIn() && hasLoopOut()
                   && (getLoopOut() - getLoopIn()) >= minimumLengthSeconds;
        }

        void setEnabled (bool shouldBeEnabled) { enabled.store (shouldBeEnabled && isValid(), std::memory_order_release); }
        bool isEnabled() const { return enabled.load (std::memory_order_acquire); }

        void toggle() { setEnabled (! isEnabled()); }

        void clear()
        {
            hasIn.store (false, std::memory_order_release);
            hasOut.store (false, std::memory_order_release);
            enabled.store (false, std::memory_order_release);
        }

    private:
        std::atomic<bool> hasIn { false };
        std::atomic<bool> hasOut { false };
        std::atomic<bool> enabled { false };
        std::atomic<double> loopInSeconds { 0.0 };
        std::atomic<double> loopOutSeconds { 0.0 };
    };

    /** 8 hot cues (ARCHITECTURE.md section "Hot Cues" / Phase 7).

        Behaviour: pressing an empty slot stores the current position there.
        Pressing an already-stored slot jumps playback to it. This mirrors the
        common CDJ workflow of "set once, then trigger" without needing a
        modifier key for the first implementation.
    */
    class HotCueStore
    {
    public:
        static constexpr int numSlots = 8;

        enum class TriggerAction { setAt, jumpTo };

        struct TriggerResult
        {
            TriggerAction action;
            double positionSeconds;
        };

        /** Sets the slot if empty and reports "setAt"; otherwise reports
            "jumpTo" with the stored position, without modifying the slot. The
            caller (DeckController) performs the actual seek. */
        TriggerResult trigger (int slotIndex, double currentPositionSeconds)
        {
            jassert (slotIndex >= 0 && slotIndex < numSlots);
            auto& slot = slots[(size_t) slotIndex];

            if (! slot.has_value())
            {
                slot = currentPositionSeconds;
                return { TriggerAction::setAt, currentPositionSeconds };
            }

            return { TriggerAction::jumpTo, *slot };
        }

        void clear (int slotIndex)
        {
            jassert (slotIndex >= 0 && slotIndex < numSlots);
            slots[(size_t) slotIndex].reset();
        }

        void clearAll() { slots.fill (std::nullopt); }

        bool isSet (int slotIndex) const
        {
            jassert (slotIndex >= 0 && slotIndex < numSlots);
            return slots[(size_t) slotIndex].has_value();
        }

        std::optional<double> get (int slotIndex) const
        {
            jassert (slotIndex >= 0 && slotIndex < numSlots);
            return slots[(size_t) slotIndex];
        }

    private:
        std::array<std::optional<double>, numSlots> slots;
    };

    /** Read-only snapshot of everything the UI needs to draw a frame. Taken by
        value on the UI thread from DeckController so the UI never touches
        engine/atomics directly. */
    struct DeckSnapshot
    {
        bool trackLoaded = false;
        juce::String trackName;
        bool playing = false;

        double positionSeconds = 0.0;
        double durationSeconds = 0.0;

        double basePitchPercent = 0.0;
        double bendPercent = 0.0;
        double effectiveRatio = 1.0;

        bool hasCue = false;
        double cuePositionSeconds = 0.0;

        bool hasLoopIn = false;
        bool hasLoopOut = false;
        bool loopEnabled = false;
        double loopInSeconds = 0.0;
        double loopOutSeconds = 0.0;

        std::array<std::optional<double>, HotCueStore::numSlots> hotCues;

        bool scratchMode = false;

        bool bpmAnalysisComplete = false;
        double bpm = 0.0;
    };
}

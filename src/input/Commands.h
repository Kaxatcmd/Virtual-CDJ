#pragma once

namespace vcdj
{
    /** Logical deck commands (ARCHITECTURE.md section 7.3).

        The input layer's only job is to translate a physical event (key
        press/hold/release, future MIDI/HID) into one of these. Nothing outside
        input/ should know about actual keycodes, and nothing outside deck/
        should know how a command is actually carried out.

        Continuous/held actions (pitch bend, jog/platter movement) are *not*
        represented as discrete commands here - they are begin/update/end calls
        on DeckController driven directly by the input layer's press/hold/
        release tracking, since a single enum value cannot carry "how long has
        this been held" information. See KeyboardInputManager.
    */
    enum class CommandId
    {
        playPause,
        stop,
        cue,
        returnToCue,
        loopIn,
        loopOut,
        loopToggle,
        pitchReset,
        pitchNudgeUp,
        pitchNudgeDown,
        scratchToggle,
        hotCue1, hotCue2, hotCue3, hotCue4, hotCue5, hotCue6, hotCue7, hotCue8,
    };
}

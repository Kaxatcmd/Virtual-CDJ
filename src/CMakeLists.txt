set(VCDJ_CORE_SOURCES
    core/AppPaths.h
    core/AppPaths.cpp

    audio/IAudioEngine.h
    audio/TrackAudioSource.h
    audio/TrackAudioSource.cpp
    audio/AudioEngine.h
    audio/AudioEngine.cpp

    deck/DeckTypes.h
    deck/PitchEngine.h
    deck/ScratchEngine.h
    deck/DeckController.h
    deck/DeckController.cpp

    input/Commands.h
    input/KeyboardInputManager.h
    input/KeyboardInputManager.cpp

    analysis/TrackAnalyzer.h
    analysis/TrackAnalyzer.cpp

    waveform/WaveformData.h
    waveform/WaveformView.h
    waveform/WaveformView.cpp

    ui/PlatterView.h
    ui/PlatterView.cpp
    ui/MainComponent.h
    ui/MainComponent.cpp
)

# vcdj_core contains every non-UI-entry-point translation unit (audio engine,
# deck domain, input layer, analysis, waveform + UI widgets). Both the real
# application and the unit test executable link against it, so domain logic
# tests never need a running JUCE application/message loop or an audio
# device (ARCHITECTURE.md section 20).
add_library(vcdj_core STATIC ${VCDJ_CORE_SOURCES})

target_include_directories(vcdj_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(vcdj_core PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_USE_WINDOWS_MEDIA_FORMAT=1
)

target_link_libraries(vcdj_core PUBLIC
    juce::juce_audio_utils
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_basics
    juce::juce_gui_extra
    juce::juce_gui_basics
    juce::juce_core
    juce::juce_events
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)

juce_add_gui_app(VirtualCDJ
    PRODUCT_NAME "Virtual CDJ"
    COMPANY_NAME "VirtualCDJProject"
    BUNDLE_ID "com.virtualcdjproject.virtualcdj"
    NEEDS_WEB_BROWSER FALSE
    NEEDS_CURL FALSE)

target_sources(VirtualCDJ PRIVATE ui/Main.cpp)

target_compile_definitions(VirtualCDJ PRIVATE
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:VirtualCDJ,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:VirtualCDJ,JUCE_VERSION>")

target_link_libraries(VirtualCDJ PRIVATE vcdj_core)

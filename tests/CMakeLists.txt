add_executable(VirtualCDJTests
    TestMain.cpp
    PitchEngineTests.cpp
    DeckTypesTests.cpp
    DeckControllerTests.cpp
    CommandMappingTests.cpp
    FakeAudioEngine.h
)

target_include_directories(VirtualCDJTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(VirtualCDJTests PRIVATE vcdj_core)

add_test(NAME VirtualCDJDomainTests COMMAND VirtualCDJTests)

cmake_minimum_required(VERSION 3.22)

project(VirtualCDJ VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# JUCE is vendored under libs/JUCE so the project builds fully offline and
# reproducibly (no network access required at configure time).
add_subdirectory(libs/JUCE)

option(VCDJ_BUILD_TESTS "Build the VirtualCDJ unit test suite" ON)

add_subdirectory(src)

if(VCDJ_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

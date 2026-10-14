cmake_minimum_required(VERSION 3.22)

project(HeritageChannel VERSION 0.2.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if(DEFINED JUCE_DIR AND EXISTS "${JUCE_DIR}/CMakeLists.txt")
    add_subdirectory("${JUCE_DIR}" JUCE)
else()
    include(FetchContent)

    # JUCE 7 does not compile against the macOS 15 SDK because it calls an
    # API that Apple made unavailable. Use current JUCE on macOS, while
    # retaining JUCE 7 for the legacy Windows 7 build.
    if(APPLE)
        set(HERITAGE_JUCE_TAG 8.0.15)
    else()
        set(HERITAGE_JUCE_TAG 7.0.12)
    endif()

    FetchContent_Declare(juce
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG ${HERITAGE_JUCE_TAG}
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(juce)
endif()

juce_add_plugin(HeritageChannel
    COMPANY_NAME "B.E.E Audio"
    BUNDLE_ID "uk.co.brenhaudio.heritagechannel"
    PLUGIN_MANUFACTURER_CODE BrHa
    PLUGIN_CODE HrCh
    FORMATS VST3
    PRODUCT_NAME "Heritage Channel"
    DESCRIPTION "Analogue console, valve compressor and multitrack tape signal flow"
    VST3_CATEGORIES Fx Dynamics EQ)

juce_generate_juce_header(HeritageChannel)

if(APPLE)
    set_target_properties(HeritageChannel PROPERTIES
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "-"
        XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO")
endif()

target_sources(HeritageChannel PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    dsp/HeritageDSP.h)

target_include_directories(HeritageChannel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(HeritageChannel PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    $<$<PLATFORM_ID:Windows>:_WIN32_WINNT=0x0601>
    $<$<PLATFORM_ID:Windows>:WINVER=0x0601>
    $<$<PLATFORM_ID:Windows>:NTDDI_VERSION=0x06010000>)

if(MSVC)
    target_compile_options(HeritageChannel PRIVATE /W4 /permissive- /EHsc /arch:SSE2)
endif()

target_link_libraries(HeritageChannel PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)

option(HERITAGE_BUILD_TESTS "Build DSP tests" ON)
if(HERITAGE_BUILD_TESTS)
    add_executable(HeritageDSPTests tests/DSPTests.cpp)
    target_include_directories(HeritageDSPTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
endif()

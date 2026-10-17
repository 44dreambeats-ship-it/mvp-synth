cmake_minimum_required(VERSION 3.22)
project(MVP VERSION 1.0.0)

# Build on macOS with JUCE. JUCE is fetched via CPM / FetchContent.
# On your Mac: cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --config Release
include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.6)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(MVP
    COMPANY_NAME "MVP"
    PLUGIN_MANUFACTURER_CODE MVPP
    PLUGIN_CODE MVPS
    FORMATS VST3 AU
    PRODUCT_NAME "MVP"
    VERSION 1.0.0
)

target_sources(MVP PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/PresetManager.cpp
    Source/PresetBrowser.cpp
    Source/ThemeManager.cpp
    Source/ThemeControls.cpp
    Source/WavetableEngine.cpp
    Source/SynthEngine.cpp
)

target_compile_features(MVP PRIVATE cxx_std_17)
target_link_libraries(MVP PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_basics
)

juce_generate_juce_header(MVP)

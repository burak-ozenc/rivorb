cmake_minimum_required(VERSION 3.22)
project(Rivorb VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(C:/JUCE JUCE)

juce_add_plugin(Rivorb
        # Identity
        PLUGIN_MANUFACTURER_CODE Rvrb
        PLUGIN_CODE             Rivb
        COMPANY_NAME            "Rivörb-Audio"
        COMPANY_WEBSITE         "https://burakozenc.me"
        COMPANY_EMAIL           "brk.ozenc@gmail.com"

        # Plugin info
        PLUGIN_NAME             "Rivorb"
        DESCRIPTION             "Spectral Adaptive Reverb"
        VERSION                 "1.0.0"

        FORMATS                 VST3
        PRODUCT_NAME            "Rivorb"
        IS_SYNTH                FALSE
        NEEDS_MIDI_INPUT        FALSE
        NEEDS_MIDI_OUTPUT       FALSE
        VST3_CATEGORIES         "Fx" "Reverb"
)

juce_generate_juce_header(Rivorb)

target_sources(Rivorb PRIVATE
        src/PluginProcessor.cpp
        src/PluginEditor.cpp
)

target_compile_definitions(Rivorb PUBLIC
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)

target_link_libraries(Rivorb PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
)
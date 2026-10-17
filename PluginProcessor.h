#pragma once
// MVP (Melodic Vibration Processor) — processor.
// Owns PresetManager (JSON-first). Presets folder: ~/Documents/MVP/Presets/
// Factory banks are copied in on first run. Drag-drop import converts .fxp/.fxb to JSON.
#ifdef __APPLE__
#include <JuceHeader.h>
#include "PresetManager.h"
#include "SynthEngine.h"

class MVPProcessor : public juce::AudioProcessor {
public:
    MVPProcessor();
    ~MVPProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "MVP"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "MVP"; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Presets
    PresetManager& presetManager() { return presetManager_; }
    // Current synth state as a single-preset JSON object (for Save Preset).
    std::string currentPresetJson() const;
    // Load a preset JSON object (from browser / drag-drop).
    void loadPresetJson(const std::string& presetJson);
    bool saveCurrentPreset(const juce::String& name, const juce::String& bankName);

private:
    SynthEngine engine_;
    PresetManager presetManager_;
    double sampleRate_ = 44100.0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MVPProcessor)
};
#else
// Non-Apple (Linux test) stub — PresetManager is tested without JUCE.
#include "PresetManager.h"
#endif

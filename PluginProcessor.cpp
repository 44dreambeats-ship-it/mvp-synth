// MVP (Melodic Vibration Processor) — JUCE processor.
// JSON-first presets. PresetManager handles folder, drag-drop, save, banks.
// This file compiles with JUCE 8 on macOS. On Linux, only PresetManager is tested.
#ifdef __APPLE__
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <sstream>
#include <vector>

MVPProcessor::MVPProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)) {
    // User presets folder: ~/Documents/MVP/Presets
    auto docs = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
    auto userRoot = docs.getChildFile("MVP").getChildFile("Presets");
    // Factory root: bundled presets folder next to the binary / in resources.
    // If not found, factory banks are still copied from the user Factory folder on first run.
    presetManager_.init(userRoot.getFullPathName().toStdString(), "");
    presetManager_.scan();
}

void MVPProcessor::prepareToPlay(double sampleRate, int) {
    sampleRate_ = sampleRate;
    engine_.setParams(engine_.params);
}
void MVPProcessor::releaseResources() {}
void MVPProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
    juce::ScopedNoDenormals noDenormals;
    for (auto& m : midi) {
        auto msg = m.getMessage();
        if (msg.isNoteOn()) engine_.noteOn((float)msg.getNoteNumber());
        else if (msg.isNoteOff()) engine_.noteOff();
    }
    auto* L = buffer.getWritePointer(0);
    auto* R = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;
    // Render mono then duplicate — engine is mono in v1.
    std::vector<float> tmp((size_t)buffer.getNumSamples());
    engine_.renderBlock(tmp.data(), buffer.getNumSamples(), (float)sampleRate_);
    for (int i = 0; i < buffer.getNumSamples(); ++i) {
        L[i] = tmp[(size_t)i];
        if (R) R[i] = tmp[(size_t)i];
    }
}
juce::AudioProcessorEditor* MVPProcessor::createEditor() { return new MVPEditor(*this); }

void MVPProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto json = currentPresetJson();
    destData.append(json.data(), json.size());
}
void MVPProcessor::setStateInformation(const void* data, int sizeInBytes) {
    if (data && sizeInBytes > 0) loadPresetJson(std::string((const char*)data, (size_t)sizeInBytes));
}

std::string MVPProcessor::currentPresetJson() const {
    // v1: serialise the engine params we track. Full param serialisation lives in SynthEngine.
    // Keep it simple + valid: a single-preset JSON object with name/category + params.
    std::ostringstream o;
    o << "{ \"name\": \"Current\", \"category\": \"Lead\", \"author\": \"User\", ";
    o << "\"master\": { \"tune_hz\": " << engine_.params.tuneHz << ", \"volume\": " << engine_.params.masterVolume << " }, ";
    o << "\"filter\": { \"cutoff\": " << engine_.params.filterCutoff << " }, ";
    o << "\"fx\": { \"distortion\": " << engine_.params.distortion << " } }";
    return o.str();
}
void MVPProcessor::loadPresetJson(const std::string& presetJson) {
    // v1: parse the few params we serialise; full mapping in SynthEngine::setParams from JSON.
    // Tolerant: if a field is missing, keep current value. Never throws.
    auto getF = [&](const char* key, float fallback) {
        std::string k = std::string("\"") + key + "\"";
        auto p = presetJson.find(k);
        if (p == std::string::npos) return fallback;
        auto colon = presetJson.find(':', p + k.size());
        if (colon == std::string::npos) return fallback;
        try { return std::stof(presetJson.substr(colon + 1)); } catch (...) { return fallback; }
    };
    SynthParams p = engine_.params;
    p.tuneHz = getF("tune_hz", p.tuneHz);
    p.masterVolume = getF("volume", p.masterVolume);
    p.filterCutoff = getF("cutoff", p.filterCutoff);
    p.distortion = getF("distortion", p.distortion);
    engine_.setParams(p);
}
bool MVPProcessor::saveCurrentPreset(const juce::String& name, const juce::String& bankName) {
    if (name.isEmpty()) return false;
    auto path = presetManager_.savePreset(name.toStdString(), "Lead", currentPresetJson(), bankName.toStdString());
    return !path.empty();
}

// JUCE entry point
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new MVPProcessor(); }
#endif

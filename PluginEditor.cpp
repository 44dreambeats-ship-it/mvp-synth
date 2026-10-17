// MVP Editor — JUCE. Preset browser is the tall sidebar (locked 2nd look).
// Drag & drop presets/banks onto the browser. Save / New Bank / Import / Open Folder in-browser.
#ifdef __APPLE__
#include "PluginEditor.h"

MVPEditor::MVPEditor(MVPProcessor& p)
    : AudioProcessorEditor(&p), proc_(p), browser_(p.presetManager()) {
    setSize(1100, 700);
    browser_.onPresetSelected = [this](const std::string& json) {
        proc_.loadPresetJson(json);
        browser_.setCurrentPresetJson(proc_.currentPresetJson());
    };
    browser_.setCurrentPresetJson(proc_.currentPresetJson());
    addAndMakeVisible(browser_);
}
void MVPEditor::paint(juce::Graphics& g) { g.fillAll(juce::Colour(0xff0e0f0c)); }
void MVPEditor::resized() { browser_.setBounds(getLocalBounds()); }
#endif

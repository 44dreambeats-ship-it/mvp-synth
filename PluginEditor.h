#pragma once
#ifdef __APPLE__
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PresetBrowser.h"

class MVPEditor : public juce::AudioProcessorEditor {
public:
    explicit MVPEditor(MVPProcessor& p);
    ~MVPEditor() override = default;
    void paint(juce::Graphics& g) override;
    void resized() override;
private:
    MVPProcessor& proc_;
    PresetBrowser browser_;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MVPEditor)
};
#endif

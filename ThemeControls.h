#pragma once
// MVP Theme Controls — JUCE UI for the Theme button.
// Top bar: [THEME 🎨] button cycles modes. Long-press / dropdown picks mode + options:
//  Manual: preset picker (MVP / Midnight / Dawn / Day / Dusk / Forest)
//  Audio: sensitivity slider (how much sound moves color)
//  Time: shows current local time + day/night gradient preview, auto-follows clock
//  Weather: condition picker (Clear/Cloudy/Rain/Snow/Thunder/Fog) + Auto (Open-Meteo, no key)
//  Timer: duration picker (15/30/60/90 min) + Start/Pause, progress bar
// All changes go through ThemeManager::update() — gradual, never a shock.
// Only compiled on macOS JUCE build.
#ifdef __APPLE__
#include <JuceHeader.h>
#include "ThemeManager.h"

class ThemeControls : public juce::Component, public juce::Button::Listener, public juce::ComboBox::Listener, public juce::Slider::Listener, public juce::Timer {
public:
    explicit ThemeControls(ThemeManager& tm);
    void resized() override;
    void paint(juce::Graphics& g) override;
    void buttonClicked(juce::Button* b) override;
    void comboBoxChanged(juce::ComboBox* cb) override;
    void sliderValueChanged(juce::Slider* s) override;
    void timerCallback() override; // 30fps: update ThemeManager + repaint preview
    std::function<void(const ThemeColors&)> onColorsChanged;
private:
    ThemeManager& tm_;
    juce::TextButton themeBtn_{ "🎨 THEME" };
    juce::ComboBox modeBox_;
    juce::ComboBox manualPresetBox_;
    juce::ComboBox weatherBox_;
    juce::Slider audioSens_{ juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };
    juce::Slider timerDur_{ juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };
    juce::TextButton timerStartBtn_{ "Start Timer" };
    juce::Label infoLabel_;
    float audioSensVal_ = 1.0f;
    float timerElapsed_ = 0; bool timerRunning_ = false;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ThemeControls)
};
#endif

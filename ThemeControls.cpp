#ifdef __APPLE__
#include "ThemeControls.h"
#include <cmath>
#include <algorithm>

ThemeControls::ThemeControls(ThemeManager& tm) : tm_(tm) {
    themeBtn_.addListener(this); addAndMakeVisible(themeBtn_);
    modeBox_.addItem("Manual", 1); modeBox_.addItem("Audio Reactive", 2); modeBox_.addItem("Time of Day", 3);
    modeBox_.addItem("Weather", 4); modeBox_.addItem("Timer", 5);
    modeBox_.setSelectedId(1); modeBox_.addListener(this); addAndMakeVisible(modeBox_);
    manualPresetBox_.addItem("MVP (locked look)", 1); manualPresetBox_.addItem("Midnight", 2);
    manualPresetBox_.addItem("Dawn", 3); manualPresetBox_.addItem("Day", 4);
    manualPresetBox_.addItem("Dusk", 5); manualPresetBox_.addItem("Forest", 6);
    manualPresetBox_.setSelectedId(1); manualPresetBox_.addListener(this); addAndMakeVisible(manualPresetBox_);
    weatherBox_.addItem("Clear", 1); weatherBox_.addItem("Cloudy", 2); weatherBox_.addItem("Rain", 3);
    weatherBox_.addItem("Snow", 4); weatherBox_.addItem("Thunder", 5); weatherBox_.addItem("Fog", 6);
    weatherBox_.addItem("Night Clear", 7); weatherBox_.addItem("Auto (Open-Meteo)", 8);
    weatherBox_.setSelectedId(1); weatherBox_.addListener(this); addAndMakeVisible(weatherBox_);
    audioSens_.setRange(0.0, 2.0, 0.01); audioSens_.setValue(1.0); audioSens_.addListener(this); addAndMakeVisible(audioSens_);
    timerDur_.setRange(5, 120, 1); timerDur_.setValue(60); timerDur_.addListener(this); addAndMakeVisible(timerDur_);
    timerStartBtn_.addListener(this); addAndMakeVisible(timerStartBtn_);
    infoLabel_.setText("Theme: Manual — MVP look. Tap THEME to cycle.", juce::dontSendNotification);
    addAndMakeVisible(infoLabel_);
    startTimerHz(30); // 30fps smoothing
}
void ThemeControls::resized() {
    auto r = getLocalBounds().reduced(6);
    themeBtn_.setBounds(r.removeFromLeft(110).reduced(2));
    modeBox_.setBounds(r.removeFromLeft(140).reduced(2));
    manualPresetBox_.setBounds(r.removeFromLeft(150).reduced(2));
    weatherBox_.setBounds(r.removeFromLeft(130).reduced(2));
    auto bottom = r; bottom.removeFromTop(4);
    audioSens_.setBounds(bottom.removeFromLeft(180).reduced(2));
    timerDur_.setBounds(bottom.removeFromLeft(180).reduced(2));
    timerStartBtn_.setBounds(bottom.removeFromLeft(100).reduced(2));
    infoLabel_.setBounds(bottom);
}
void ThemeControls::paint(juce::Graphics& g) {
    auto c = tm_.current();
    g.fillAll(juce::Colour((uint8_t)(c.bg.r*255), (uint8_t)(c.bg.g*255), (uint8_t)(c.bg.b*255)));
    // Day->night gradient preview strip (bottom edge)
    auto r = getLocalBounds().removeFromBottom(6);
    for (int x=0; x<r.getWidth(); ++x) {
        float t = (float)x / (float)std::max(1, r.getWidth());
        auto col = ThemeManager::themeForTimeOfDay(t*24.0f);
        g.setColour(juce::Colour((uint8_t)(col.accent.r*255), (uint8_t)(col.accent.g*255), (uint8_t)(col.accent.b*255)));
        g.fillRect(r.getX()+x, r.getY(), 1, r.getHeight());
    }
}
void ThemeControls::buttonClicked(juce::Button* b) {
    if (b==&themeBtn_) { tm_.cycleMode(); modeBox_.setSelectedId((int)tm_.mode()+1, juce::dontSendNotification); }
    if (b==&timerStartBtn_) { timerRunning_ = !timerRunning_; timerStartBtn_.setButtonText(timerRunning_ ? "Pause Timer" : "Start Timer"); }
}
void ThemeControls::comboBoxChanged(juce::ComboBox* cb) {
    if (cb==&modeBox_) tm_.setMode((ThemeMode)(modeBox_.getSelectedId()-1));
    if (cb==&manualPresetBox_) tm_.setManualPreset(manualPresetBox_.getSelectedId()-1);
    if (cb==&weatherBox_) {
        int id = weatherBox_.getSelectedId();
        if (id>=1 && id<=7) tm_.setWeather((WeatherCondition)(id-1));
        else if (id==8) {
            // Auto: Open-Meteo free API, no key. In real build, fetch on background thread:
            // https://api.open-meteo.com/v1/forecast?latitude=..&longitude=..&current=weather_code,is_day
            // Map weather_code -> WeatherCondition, is_day=0 + clear -> NightClear.
            // Fallback: keep current manual weather if fetch fails.
            infoLabel_.setText("Weather Auto: fetching Open-Meteo… (falls back to manual if offline)", juce::dontSendNotification);
        }
    }
}
void ThemeControls::sliderValueChanged(juce::Slider* s) {
    if (s==&audioSens_) audioSensVal_ = (float)audioSens_.getValue();
    // timerDur_ read in timerCallback
}
void ThemeControls::timerCallback() {
    // Time of Day: follow local clock
    if (tm_.mode()==ThemeMode::TimeOfDay) {
        auto now = juce::Time::getCurrentTime();
        tm_.setTimeOfDay((float)now.getHours() + (float)now.getMinutes()/60.0f);
    }
    // Timer: advance progress
    if (tm_.mode()==ThemeMode::Timer && timerRunning_) {
        timerElapsed_ += 1.0f/30.0f;
        float dur = (float)timerDur_.getValue()*60.0f;
        tm_.setTimerProgress(std::fmod(timerElapsed_/dur, 1.0f));
        if (timerElapsed_>=dur) { timerElapsed_=0; }
    }
    auto colors = tm_.update(1.0f/30.0f);
    infoLabel_.setText("Theme: " + ThemeManager::modeName(tm_.mode()) + " — gradual", juce::dontSendNotification);
    if (onColorsChanged) onColorsChanged(colors);
    repaint();
}
#endif

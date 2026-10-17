#pragma once
// MVP ThemeManager — color scheme + Theme button.
// Modes: Manual (locked 2nd look default) / AudioReactive / TimeOfDay / Weather / Timer.
// RULE: every change is GRADUAL. No hard cuts. Colors are smoothed every frame:
//   current = lerp(current, target, 1 - exp(-dt / tau))
// Day->Night is interpolated between keyframes with cosine easing, so as time
// moves slowly, color moves slowly. Weather / Audio / Timer all lerp to their target.
// No JUCE dependency — pure color math, testable. JUCE editor polls currentColors().
#include <string>
#include <vector>

struct RGB { float r=0, g=0, b=0; };
struct ThemeColors {
    RGB bg{0.055f, 0.06f, 0.047f};       // off-black
    RGB panel{0.09f, 0.10f, 0.08f};
    RGB accent{0.71f, 1.0f, 0.18f};      // acid lime
    RGB highlight{1.0f, 0.16f, 0.24f};   // hot red
    RGB text{0.95f, 0.95f, 0.95f};
    RGB textDim{0.65f, 0.65f, 0.65f};
    RGB visualizer{0.71f, 1.0f, 0.18f};
};

enum class ThemeMode { Manual=0, AudioReactive=1, TimeOfDay=2, Weather=3, Timer=4 };
enum class WeatherCondition { Clear=0, Cloudy=1, Rain=2, Snow=3, Thunder=4, Fog=5, NightClear=6 };

struct ThemeKeyframe { float hour; ThemeColors colors; }; // hour 0-24

class ThemeManager {
public:
    ThemeManager();

    // --- Mode ---
    void setMode(ThemeMode m) { mode_ = m; }
    ThemeMode mode() const { return mode_; }
    void cycleMode(); // Theme button: Manual -> Audio -> Time -> Weather -> Timer -> Manual
    static std::string modeName(ThemeMode m);

    // --- Manual ---
    void setManualTheme(const ThemeColors& c) { manual_ = c; }
    ThemeColors manualTheme() const { return manual_; }
    void setManualPreset(int idx); // 0 MVP (locked 2nd look), 1 Midnight, 2 Dawn, 3 Day, 4 Dusk, 5 Forest

    // --- Inputs (set each frame / on change) ---
    void setAudioLevel(float rms01, float pitchNorm01=0.5f); // 0..1
    void setWeather(WeatherCondition w) { weather_ = w; }
    void setTimeOfDay(float hour24); // 0..24, from local clock
    void setTimerProgress(float p01, float durationMinutes=60.0f); // 0..1 through timer cycle

    // --- Update: call every frame with dt seconds. Returns smoothed colors. ---
    ThemeColors update(float dtSeconds);

    ThemeColors current() const { return current_; }
    ThemeColors target() const { return target_; }

    // --- Pure helpers (tested) ---
    static ThemeColors lerpColors(const ThemeColors& a, const ThemeColors& b, float t);
    static RGB lerpRGB(RGB a, RGB b, float t);
    static float smoothstep(float t); // cosine easing 0..1
    static ThemeColors themeForTimeOfDay(float hour24); // keyframe interpolation
    static ThemeColors themeForWeather(WeatherCondition w);
    static ThemeColors themeForAudio(float rms01, float pitchNorm01, const ThemeColors& base);
    static ThemeColors themeForTimer(float p01);
    static RGB hslToRgb(float h, float s, float l);
    static void rgbToHsl(RGB c, float& h, float& s, float& l);
    static float maxStepPerFrame(const ThemeColors& a, const ThemeColors& b); // max channel delta

    // Smoothing time constants (seconds). Larger = slower / more gradual.
    float tauAudio = 0.8f;    // audio: smooth but responsive
    float tauWeather = 5.0f;  // weather: slow drift
    float tauTime = 30.0f;    // day/night: very gradual (plus keyframe interpolation)
    float tauTimer = 3.0f;
    float tauManual = 1.2f;   // manual switch: ~1s fade, not a shock

private:
    ThemeMode mode_ = ThemeMode::Manual;
    ThemeColors manual_;
    ThemeColors current_;
    ThemeColors target_;
    WeatherCondition weather_ = WeatherCondition::Clear;
    float hour24_ = 12.0f;
    float audioRms_ = 0.0f;
    float audioPitch_ = 0.5f;
    float timerP_ = 0.0f;

    ThemeColors computeTarget() const;
    float tauForMode() const;
};

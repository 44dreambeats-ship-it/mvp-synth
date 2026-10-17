// g++ -std=c++17 Source/ThemeManager.cpp tests/test_theme_manager.cpp -o /tmp/test_theme && /tmp/test_theme
#include "../Source/ThemeManager.h"
#include <cstdio>
#include <cassert>
#include <cmath>
int main(){
    ThemeManager tm;
    // 1. Manual default = MVP locked look (acid lime)
    auto c0 = tm.current();
    assert(c0.accent.g > 0.9f && c0.accent.r > 0.6f);
    printf("default MVP accent OK\n");
    // 2. Gradual: single update must not jump — max step small
    tm.setMode(ThemeMode::Manual);
    tm.setManualPreset(3); // Day (light) — big target jump
    auto before = tm.current();
    auto after = tm.update(0.016f); // 1 frame @60fps
    float step = ThemeManager::maxStepPerFrame(before, after);
    printf("1-frame step to Day: %.5f (must be < 0.05)\n", step);
    assert(step < 0.05f);
    // After many frames, it converges (gradual, not instant)
    for(int i=0;i<600;i++) tm.update(0.016f);
    auto converged = tm.current();
    float dist = ThemeManager::maxStepPerFrame(converged, tm.target());
    printf("after 10s dist to target: %.4f (must be < 0.05)\n", dist);
    assert(dist < 0.05f);
    // 3. Time-of-day: smooth interpolation — 11:59 vs 12:01 must be close
    auto t1 = ThemeManager::themeForTimeOfDay(11.99f);
    auto t2 = ThemeManager::themeForTimeOfDay(12.01f);
    float d = ThemeManager::maxStepPerFrame(t1,t2);
    printf("day/noon continuity 11:59->12:01: %.5f (must be < 0.02)\n", d);
    assert(d < 0.02f);
    // Night vs noon are different
    auto night = ThemeManager::themeForTimeOfDay(0.0f);
    auto noon = ThemeManager::themeForTimeOfDay(12.0f);
    assert(ThemeManager::maxStepPerFrame(night,noon) > 0.3f);
    printf("night vs noon differ OK\n");
    // 4. Weather: each condition gives distinct but valid colors
    for(int i=0;i<7;i++){
        auto w = ThemeManager::themeForWeather((WeatherCondition)i);
        assert(w.accent.r>=0 && w.accent.r<=1 && w.bg.r>=0 && w.bg.r<=1);
    }
    printf("weather OK\n");
    // 5. Audio: louder -> brighter, pitch shifts hue, still smooth
    auto base = ThemeColors{};
    auto quiet = ThemeManager::themeForAudio(0.0f, 0.5f, base);
    auto loud = ThemeManager::themeForAudio(1.0f, 0.5f, base);
    assert(ThemeManager::maxStepPerFrame(quiet,loud) > 0.01f);
    assert(ThemeManager::maxStepPerFrame(quiet,loud) < 0.4f); // not a shock
    printf("audio OK\n");
    // 6. Timer: 0 vs 0.01 close (gradual), 0 vs 0.5 different
    auto tm0 = ThemeManager::themeForTimer(0.0f);
    auto tm001 = ThemeManager::themeForTimer(0.01f);
    assert(ThemeManager::maxStepPerFrame(tm0,tm001) < 0.1f);
    printf("timer gradual OK\n");
    // 7. Cycle mode
    tm.setMode(ThemeMode::Manual); tm.cycleMode(); assert(tm.mode()==ThemeMode::AudioReactive);
    printf("cycle OK\n");
    printf("PASS\n");
    return 0;
}

#include "ThemeManager.h"
#include <cmath>
#include <algorithm>

static float clamp01(float v){ return v<0?0:(v>1?1:v); }
static float lerpF(float a, float b, float t){ return a+(b-a)*t; }

RGB ThemeManager::lerpRGB(RGB a, RGB b, float t){
    t=clamp01(t);
    return { lerpF(a.r,b.r,t), lerpF(a.g,b.g,t), lerpF(a.b,b.b,t) };
}
ThemeColors ThemeManager::lerpColors(const ThemeColors& a, const ThemeColors& b, float t){
    t=clamp01(t);
    ThemeColors o;
    o.bg=lerpRGB(a.bg,b.bg,t); o.panel=lerpRGB(a.panel,b.panel,t);
    o.accent=lerpRGB(a.accent,b.accent,t); o.highlight=lerpRGB(a.highlight,b.highlight,t);
    o.text=lerpRGB(a.text,b.text,t); o.textDim=lerpRGB(a.textDim,b.textDim,t);
    o.visualizer=lerpRGB(a.visualizer,b.visualizer,t);
    return o;
}
float ThemeManager::smoothstep(float t){
    t=clamp01(t);
    return 0.5f - 0.5f * std::cos(t * 3.14159265f); // cosine easing, zero slope at ends
}
float ThemeManager::maxStepPerFrame(const ThemeColors& a, const ThemeColors& b){
    auto m=[](RGB x, RGB y){
        return std::max({std::abs(x.r-y.r), std::abs(x.g-y.g), std::abs(x.b-y.b)});
    };
    return std::max({m(a.bg,b.bg), m(a.panel,b.panel), m(a.accent,b.accent), m(a.highlight,b.highlight), m(a.text,b.text), m(a.visualizer,b.visualizer)});
}
RGB ThemeManager::hslToRgb(float h, float s, float l){
    h = std::fmod(h,1.0f); if(h<0) h+=1.0f; s=clamp01(s); l=clamp01(l);
    auto hue2=[&](float p,float q,float t){
        if(t<0) t+=1; if(t>1) t-=1;
        if(t<1.0f/6) return p+(q-p)*6*t;
        if(t<1.0f/2) return q;
        if(t<2.0f/3) return p+(q-p)*(2.0f/3-t)*6;
        return p;
    };
    if(s==0) return {l,l,l};
    float q = l<0.5f ? l*(1+s) : l+s-l*s;
    float p = 2*l-q;
    return { hue2(p,q,h+1.0f/3), hue2(p,q,h), hue2(p,q,h-1.0f/3) };
}
void ThemeManager::rgbToHsl(RGB c, float& h, float& s, float& l){
    float mx=std::max({c.r,c.g,c.b}), mn=std::min({c.r,c.g,c.b});
    l=(mx+mn)/2;
    if(mx==mn){ h=0; s=0; return; }
    float d=mx-mn;
    s = l>0.5f ? d/(2-mx-mn) : d/(mx+mn);
    if(mx==c.r) h=(c.g-c.b)/d + (c.g<c.b?6:0);
    else if(mx==c.g) h=(c.b-c.r)/d+2;
    else h=(c.r-c.g)/d+4;
    h/=6;
}

// --- Presets ---
ThemeManager::ThemeManager(){
    // Locked 2nd look = default manual
    manual_ = ThemeColors{};
    current_ = manual_;
    target_ = manual_;
}
std::string ThemeManager::modeName(ThemeMode m){
    switch(m){
        case ThemeMode::Manual: return "Manual";
        case ThemeMode::AudioReactive: return "Audio Reactive";
        case ThemeMode::TimeOfDay: return "Time of Day";
        case ThemeMode::Weather: return "Weather";
        case ThemeMode::Timer: return "Timer";
    }
    return "Manual";
}
void ThemeManager::cycleMode(){
    int n=(int)mode_+1; if(n>4) n=0; mode_=(ThemeMode)n;
}
void ThemeManager::setManualPreset(int idx){
    ThemeColors c = ThemeColors{};
    switch(idx){
        case 0: c=ThemeColors{}; break; // MVP locked 2nd look
        case 1: // Midnight — deep blue-black, cyan accent
            c.bg={0.03f,0.04f,0.09f}; c.panel={0.06f,0.07f,0.14f}; c.accent={0.2f,0.85f,1.0f}; c.highlight={0.6f,0.4f,1.0f}; c.visualizer={0.2f,0.85f,1.0f}; break;
        case 2: // Dawn — warm peach / soft pink
            c.bg={0.16f,0.11f,0.13f}; c.panel={0.22f,0.16f,0.18f}; c.accent={1.0f,0.65f,0.35f}; c.highlight={1.0f,0.4f,0.55f}; c.visualizer={1.0f,0.65f,0.35f}; break;
        case 3: // Day — light, high contrast
            c.bg={0.93f,0.93f,0.90f}; c.panel={0.85f,0.85f,0.82f}; c.accent={0.15f,0.6f,0.25f}; c.highlight={0.9f,0.2f,0.15f}; c.text={0.1f,0.1f,0.1f}; c.textDim={0.4f,0.4f,0.4f}; c.visualizer={0.15f,0.6f,0.25f}; break;
        case 4: // Dusk — purple / orange sunset
            c.bg={0.10f,0.05f,0.12f}; c.panel={0.16f,0.08f,0.18f}; c.accent={1.0f,0.5f,0.15f}; c.highlight={0.75f,0.25f,0.85f}; c.visualizer={1.0f,0.5f,0.15f}; break;
        case 5: // Forest — deep green
            c.bg={0.04f,0.08f,0.05f}; c.panel={0.07f,0.12f,0.08f}; c.accent={0.45f,0.9f,0.35f}; c.highlight={0.95f,0.85f,0.2f}; c.visualizer={0.45f,0.9f,0.35f}; break;
        default: break;
    }
    manual_=c;
}

// --- Inputs ---
void ThemeManager::setAudioLevel(float rms01, float pitchNorm01){ audioRms_=clamp01(rms01); audioPitch_=clamp01(pitchNorm01); }
void ThemeManager::setTimeOfDay(float h){ hour24_=std::fmod(h,24.0f); if(hour24_<0) hour24_+=24; }
void ThemeManager::setTimerProgress(float p, float){ timerP_=clamp01(p); }

// --- Target builders ---
ThemeColors ThemeManager::themeForTimeOfDay(float hour24){
    // Keyframes: Night 0h, Dawn 6h, Noon 12h, Dusk 18h, Night 24h. Cosine interpolation between them.
    struct K{ float h; ThemeColors c; };
    ThemeColors night; night.bg={0.03f,0.04f,0.08f}; night.panel={0.06f,0.07f,0.12f}; night.accent={0.3f,0.5f,1.0f}; night.highlight={0.6f,0.4f,1.0f}; night.visualizer={0.3f,0.5f,1.0f}; night.text={0.9f,0.9f,0.95f}; night.textDim={0.55f,0.55f,0.65f};
    ThemeColors dawn; dawn.bg={0.16f,0.11f,0.13f}; dawn.panel={0.22f,0.16f,0.18f}; dawn.accent={1.0f,0.65f,0.35f}; dawn.highlight={1.0f,0.4f,0.55f}; dawn.visualizer={1.0f,0.65f,0.35f};
    ThemeColors noon; noon.bg={0.93f,0.93f,0.90f}; noon.panel={0.85f,0.85f,0.82f}; noon.accent={0.15f,0.6f,0.25f}; noon.highlight={0.9f,0.2f,0.15f}; noon.text={0.1f,0.1f,0.1f}; noon.textDim={0.4f,0.4f,0.4f}; noon.visualizer={0.15f,0.6f,0.25f};
    ThemeColors dusk; dusk.bg={0.10f,0.05f,0.12f}; dusk.panel={0.16f,0.08f,0.18f}; dusk.accent={1.0f,0.5f,0.15f}; dusk.highlight={0.75f,0.25f,0.85f}; dusk.visualizer={1.0f,0.5f,0.15f};
    std::vector<K> ks={{0,night},{6,dawn},{12,noon},{18,dusk},{24,night}};
    float h=std::fmod(hour24,24.0f); if(h<0) h+=24;
    for(size_t i=0;i+1<ks.size();++i){
        if(h>=ks[i].h && h<=ks[i+1].h){
            float t=(h-ks[i].h)/(ks[i+1].h-ks[i].h);
            return lerpColors(ks[i].c, ks[i+1].c, smoothstep(t));
        }
    }
    return night;
}
ThemeColors ThemeManager::themeForWeather(WeatherCondition w){
    ThemeColors base = ThemeColors{};
    switch(w){
        case WeatherCondition::Clear: base.bg={0.08f,0.09f,0.12f}; base.panel={0.12f,0.13f,0.17f}; base.accent={1.0f,0.8f,0.25f}; base.highlight={1.0f,0.5f,0.15f}; base.visualizer={1.0f,0.8f,0.25f}; break;
        case WeatherCondition::Cloudy: base.bg={0.12f,0.13f,0.14f}; base.panel={0.18f,0.19f,0.20f}; base.accent={0.6f,0.7f,0.8f}; base.highlight={0.4f,0.55f,0.7f}; base.visualizer={0.6f,0.7f,0.8f}; break;
        case WeatherCondition::Rain: base.bg={0.05f,0.08f,0.14f}; base.panel={0.08f,0.12f,0.20f}; base.accent={0.25f,0.65f,1.0f}; base.highlight={0.5f,0.8f,1.0f}; base.visualizer={0.25f,0.65f,1.0f}; break;
        case WeatherCondition::Snow: base.bg={0.88f,0.90f,0.95f}; base.panel={0.78f,0.82f,0.88f}; base.accent={0.3f,0.6f,0.9f}; base.highlight={0.9f,0.3f,0.4f}; base.text={0.1f,0.12f,0.18f}; base.textDim={0.45f,0.48f,0.55f}; base.visualizer={0.3f,0.6f,0.9f}; break;
        case WeatherCondition::Thunder: base.bg={0.06f,0.04f,0.12f}; base.panel={0.10f,0.07f,0.18f}; base.accent={0.75f,0.6f,1.0f}; base.highlight={1.0f,0.85f,0.2f}; base.visualizer={0.75f,0.6f,1.0f}; break;
        case WeatherCondition::Fog: base.bg={0.15f,0.15f,0.14f}; base.panel={0.20f,0.20f,0.19f}; base.accent={0.75f,0.75f,0.70f}; base.highlight={0.6f,0.6f,0.55f}; base.visualizer={0.75f,0.75f,0.70f}; break;
        case WeatherCondition::NightClear: base.bg={0.03f,0.04f,0.09f}; base.panel={0.06f,0.07f,0.14f}; base.accent={0.5f,0.7f,1.0f}; base.highlight={0.9f,0.7f,0.3f}; base.visualizer={0.5f,0.7f,1.0f}; break;
    }
    return base;
}
ThemeColors ThemeManager::themeForAudio(float rms01, float pitchNorm01, const ThemeColors& base){
    // Hue shifts with pitch, brightness/energy with RMS. Small, smooth shifts — not strobing.
    float h,s,l; rgbToHsl(base.accent,h,s,l);
    float newH = h + (pitchNorm01-0.5f)*0.12f; // ± ~43° max
    float newL = clamp01(l + rms01*0.15f);
    ThemeColors o=base;
    o.accent = hslToRgb(newH, s, newL);
    o.visualizer = hslToRgb(newH, s, clamp01(newL+0.05f));
    // Background breathes slightly with level
    o.bg = lerpRGB(base.bg, {base.bg.r+0.02f*rms01, base.bg.g+0.02f*rms01, base.bg.b+0.03f*rms01}, 0.5f);
    o.highlight = hslToRgb(newH+0.08f, s, clamp01(l+0.05f+rms01*0.1f));
    return o;
}
ThemeColors ThemeManager::themeForTimer(float p01){
    // Slow rainbow drift over the timer cycle — hue rotates 0..1 over full cycle.
    ThemeColors base = ThemeColors{};
    float h = p01; // 0..1
    base.accent = hslToRgb(h, 0.85f, 0.55f);
    base.visualizer = hslToRgb(h, 0.85f, 0.55f);
    base.highlight = hslToRgb(std::fmod(h+0.08f,1.0f), 0.85f, 0.55f);
    // BG stays dark for readability
    base.bg = {0.055f+0.02f*std::sin(p01*6.28f), 0.06f, 0.047f+0.02f*std::cos(p01*6.28f)};
    return base;
}
ThemeColors ThemeManager::computeTarget() const{
    switch(mode_){
        case ThemeMode::Manual: return manual_;
        case ThemeMode::AudioReactive: return themeForAudio(audioRms_, audioPitch_, manual_);
        case ThemeMode::TimeOfDay: return themeForTimeOfDay(hour24_);
        case ThemeMode::Weather: return themeForWeather(weather_);
        case ThemeMode::Timer: return themeForTimer(timerP_);
    }
    return manual_;
}
float ThemeManager::tauForMode() const{
    switch(mode_){
        case ThemeMode::Manual: return tauManual;
        case ThemeMode::AudioReactive: return tauAudio;
        case ThemeMode::TimeOfDay: return tauTime;
        case ThemeMode::Weather: return tauWeather;
        case ThemeMode::Timer: return tauTimer;
    }
    return tauManual;
}
ThemeColors ThemeManager::update(float dt){
    if(dt<=0) dt=0.016f;
    if(dt>1.0f) dt=1.0f;
    target_=computeTarget();
    float tau=tauForMode();
    float k = 1.0f - std::exp(-dt/tau); // exponential smoothing — never jumps
    current_ = lerpColors(current_, target_, k);
    return current_;
}

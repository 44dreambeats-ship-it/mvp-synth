#include "WavetableEngine.h"
#include <algorithm>
static float clampf(float v, float lo, float hi){ return std::max(lo, std::min(hi, v)); }
void WavetableOsc::build(const std::string& type, int numFrames, int size){
    frames.assign(numFrames, std::vector<float>(size, 0.0f));
    for(int f=0; f<numFrames; ++f){
        float morph = (float)f / std::max(1, numFrames-1);
        for(int i=0; i<size; ++i){
            float ph = (float)i / size;
            float v = 0.0f;
            if(type=="Saw"||type=="HarshSaw"||type=="Supersaw"||type=="BitSaw") v = 2.0f*ph-1.0f;
            else if(type=="Square"||type=="Pulse") v = ph<0.5f?1.0f:-1.0f;
            else if(type=="Sine") v = std::sin(2.0f*M_PI*ph);
            else if(type=="Triangle") v = 2.0f*std::abs(2.0f*ph-1.0f)-1.0f;
            else if(type=="FM_Bell") v = std::sin(2.0f*M_PI*ph) * std::sin(2.0f*M_PI*ph*3.01f);
            else v = std::sin(2.0f*M_PI*ph);
            // morph: blend toward a brighter variant as frame increases
            float bright = std::sin(2.0f*M_PI*ph*3.0f)*0.3f + v*0.7f;
            frames[f][i] = v*(1.0f-morph) + bright*morph;
        }
    }
}
float WavetableOsc::next(float freq, float sampleRate, float wtPos, float warp, const std::string& warpMode){
    if(frames.empty()) return 0.0f;
    phase += freq / sampleRate;
    if(phase>=1.0f) phase-=1.0f;
    float pos = clampf(wtPos,0.0f,1.0f)*(frames.size()-1);
    int f0=(int)pos; int f1=std::min((int)frames.size()-1, f0+1); float frac=pos-f0;
    int size=(int)frames[0].size();
    float idx = phase*size;
    int i0=(int)idx; int i1=(i0+1)%size; float ifrac=idx-i0;
    auto sampleAt=[&](int fr){ return frames[fr][i0]*(1.0f-ifrac)+frames[fr][i1]*ifrac; };
    float v = sampleAt(f0)*(1.0f-frac)+sampleAt(f1)*frac;
    // warp
    if(warpMode=="Bend") v = std::tanh(v*(1.0f+warp*3.0f));
    else if(warpMode=="Fold") v = std::sin(v*(1.0f+warp*4.0f));
    else if(warpMode=="PWM") v = v>warp-0.5f?1.0f:-1.0f;
    else if(warpMode=="Asym") v = v>=0? v*(1.0f-warp*0.5f) : v*(1.0f+warp*0.5f);
    return clampf(v,-1.0f,1.0f);
}
void ADSR::noteOn(){ stage=1; }
void ADSR::noteOff(){ stage=4; }
float ADSR::next(float sr){
    float dt=1.0f/sr;
    if(stage==1){ level+=dt/std::max(0.001f,attack); if(level>=1.0f){level=1.0f; stage=2;} }
    else if(stage==2){ level-=dt/std::max(0.001f,decay)*(1.0f-sustain); if(level<=sustain){level=sustain; stage=3;} }
    else if(stage==4){ level-=dt/std::max(0.001f,release)*1.0f; if(level<=0){level=0; stage=0;} }
    return level;
}
float SimpleFilter::process(float in, float sr){
    // one-pole lowpass approximation with drive + resonance feedback
    float driven = std::tanh(in*(1.0f+drive*2.0f));
    float alpha = clampf(2.0f*M_PI*cutoff/sr, 0.0f, 1.0f);
    z1 += alpha*(driven - z1);
    z2 += alpha*(z1 - z2);
    float out = z2 + (z2 - z1)*resonance*0.5f;
    return clampf(out,-1.0f,1.0f);
}
float tuneRatioForHz(float masterHz){ return masterHz/440.0f; }

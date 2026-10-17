#include "SynthEngine.h"
#include <cmath>
#include <algorithm>
static float clampf(float v,float lo,float hi){ return std::max(lo,std::min(hi,v)); }
float SynthEngine::midiToFreq(int midi, float tuneHz){
    float base = 440.0f * std::pow(2.0f, (midi-69)/12.0f);
    return base * (tuneHz/440.0f);
}
void Voice::init(const SynthParams& p){
    osc1.build(p.osc1Type); osc2.build(p.osc2Type);
    ampEnv.attack=p.ampAttack; ampEnv.decay=p.ampDecay; ampEnv.sustain=p.ampSustain; ampEnv.release=p.ampRelease;
    filtEnv.attack=p.filtAttack; filtEnv.decay=p.filtDecay; filtEnv.sustain=p.filtSustain; filtEnv.release=p.filtRelease;
    filter.cutoff=p.filterCutoff; filter.resonance=p.filterReso; filter.drive=p.filterDrive;
    delayLine.assign(44100, 0.0f); delayIdx=0;
}
void Voice::noteOn(float f){ freq=f; ampEnv.noteOn(); filtEnv.noteOn(); active=true; }
void Voice::noteOff(){ ampEnv.noteOff(); filtEnv.noteOff(); }
float Voice::render(const SynthParams& p, float sr, float time){
    if(!active) return 0.0f;
    float lfo1 = std::sin(2.0f*M_PI*lfo1Phase);
    float lfo2 = std::sin(2.0f*M_PI*lfo2Phase);
    lfo1Phase += p.lfo1Rate/sr; if(lfo1Phase>=1) lfo1Phase-=1;
    lfo2Phase += p.lfo2Rate/sr; if(lfo2Phase>=1) lfo2Phase-=1;
    float f1 = freq * std::pow(2.0f, p.osc1Octave) * (1.0f + lfo1*p.lfo1Amount*0.01f);
    float f2 = freq * std::pow(2.0f, p.osc2Octave);
    float s = 0.0f;
    s += osc1.next(f1, sr, p.osc1WtPos, p.osc1Warp, p.osc1WarpMode) * p.osc1Level;
    s += osc2.next(f2, sr, p.osc2WtPos, p.osc2Warp, p.osc2WarpMode) * p.osc2Level;
    s += std::sin(2.0f*M_PI*freq*0.5f*time) * p.subLevel * 0.5f; // sub placeholder phase via time
    // noise (deterministic pseudo-random)
    float noise = ((int)(time*sr*7919)%1000)/1000.0f*2.0f-1.0f;
    s += noise * p.noiseLevel;
    // filter with env + LFO modulation
    float filtEnvVal = filtEnv.next(sr);
    float modCutoff = p.filterCutoff * (1.0f + filtEnvVal*p.filterEnvAmount + lfo1*p.lfo1Amount*0.2f);
    filter.cutoff = clampf(modCutoff, 40.0f, 18000.0f);
    s = filter.process(s, sr);
    float amp = ampEnv.next(sr);
    s *= amp;
    // distortion + bitcrush
    if(p.distortion>0.001f) s = std::tanh(s*(1.0f+p.distortion*3.0f)) / (1.0f+p.distortion*0.5f);
    if(p.bitcrush>0.001f){
        float steps = 1.0f + (1.0f-p.bitcrush)*14.0f;
        s = std::round(s*steps)/steps;
    }
    // simple delay (250ms)
    int delaySamples = (int)(0.25f*sr);
    float delayed = delayLine[(delayIdx - delaySamples + (int)delayLine.size()) % (int)delayLine.size()];
    delayLine[delayIdx]=s; delayIdx=(delayIdx+1)%(int)delayLine.size();
    s = s*(1.0f-p.delayMix) + delayed*p.delayMix;
    // crude reverb: feedback comb
    s += delayed * p.reverbMix * 0.3f;
    if(ampEnv.stage==0 && ampEnv.level<=0.0001f) active=false;
    return clampf(s * p.masterVolume, -1.0f, 1.0f);
}
void SynthEngine::setParams(const SynthParams& p){ params=p; voice.init(p); }
void SynthEngine::noteOn(float midiNote){ voice.noteOn(midiToFreq((int)midiNote, params.tuneHz)); }
void SynthEngine::noteOff(){ voice.noteOff(); }
void SynthEngine::renderBlock(float* out, int n, float sr){
    static float t=0.0f;
    for(int i=0;i<n;++i){ out[i]=voice.render(params, sr, t); t+=1.0f/sr; }
}

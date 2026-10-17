#pragma once
#include "WavetableEngine.h"
#include <vector>
// Standalone DSP core — no JUCE dependency, testable on Linux/Mac.
// JUCE processor wraps this class. Preset JSON maps 1:1 to SynthParams.
struct SynthParams {
    float tuneHz = 440.0f;
    std::string osc1Type = "Saw", osc2Type = "Saw";
    std::string osc1WarpMode = "Bend", osc2WarpMode = "Bend";
    float osc1Level=0.9f, osc2Level=0.5f;
    float osc1WtPos=0.3f, osc2WtPos=0.5f;
    float osc1Warp=0.3f, osc2Warp=0.2f;
    int osc1Octave=0, osc2Octave=0;
    float subLevel=0.4f;
    float noiseLevel=0.05f;
    float filterCutoff=3000.0f, filterReso=0.4f, filterDrive=0.5f, filterEnvAmount=0.5f;
    float ampAttack=0.01f, ampDecay=0.2f, ampSustain=0.4f, ampRelease=0.2f;
    float filtAttack=0.01f, filtDecay=0.2f, filtSustain=0.3f, filtRelease=0.2f;
    float lfo1Rate=2.0f, lfo1Amount=0.3f;
    float lfo2Rate=0.5f, lfo2Amount=0.2f;
    float distortion=0.4f, bitcrush=0.1f, chorus=0.2f;
    float delayMix=0.15f, reverbMix=0.2f;
    float masterVolume=0.8f;
    float glide=0.0f;
};
struct Voice {
    WavetableOsc osc1, osc2;
    ADSR ampEnv, filtEnv;
    SimpleFilter filter;
    float freq=440.0f; float lfo1Phase=0, lfo2Phase=0;
    bool active=false;
    std::vector<float> delayLine; int delayIdx=0;
    void init(const SynthParams& p);
    void noteOn(float f);
    void noteOff();
    float render(const SynthParams& p, float sampleRate, float time);
};
struct SynthEngine {
    SynthParams params;
    Voice voice;
    void setParams(const SynthParams& p);
    void noteOn(float midiNote); // midi -> freq * tuneRatio
    void noteOff();
    void renderBlock(float* out, int numSamples, float sampleRate);
    static float midiToFreq(int midi, float tuneHz);
};

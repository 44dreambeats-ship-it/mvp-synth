#pragma once
#include <vector>
#include <cmath>
#include <string>
// Minimal wavetable engine — original code, no Serum code.
// Generates wavetables from formulas; morphs + warps per preset.
struct WavetableOsc {
    std::vector<std::vector<float>> frames; // [frame][sample]
    float phase = 0.0f;
    void build(const std::string& type, int numFrames = 64, int size = 2048);
    float next(float freq, float sampleRate, float wtPos, float warp, const std::string& warpMode);
};
struct ADSR {
    float attack=0.01f, decay=0.2f, sustain=0.5f, release=0.2f;
    float level=0.0f; int stage=0; // 0 idle, 1 attack, 2 decay, 3 sustain, 4 release
    void noteOn(); void noteOff();
    float next(float sampleRate);
};
struct SimpleFilter {
    float cutoff=2000.0f, resonance=0.5f, drive=0.5f;
    float z1=0, z2=0;
    float process(float in, float sampleRate);
};
float tuneRatioForHz(float masterHz); // 440 -> 1.0, 432 -> 432/440, etc.

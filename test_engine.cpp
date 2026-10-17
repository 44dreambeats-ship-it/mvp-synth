// Standalone DSP test — compiles without JUCE: g++ -std=c++17 Source/*.cpp tests/test_engine.cpp -o /tmp/test_engine
#include "../Source/SynthEngine.h"
#include <cstdio>
#include <cmath>
#include <vector>
int main(){
    SynthEngine eng;
    SynthParams p; p.tuneHz=440; p.osc1Type="Saw"; p.osc2Type="Saw"; p.filterCutoff=3000; p.distortion=0.5f; p.bitcrush=0.2f;
    eng.setParams(p);
    eng.noteOn(69); // A4
    std::vector<float> buf(44100);
    eng.renderBlock(buf.data(), (int)buf.size(), 44100.0f);
    double sumsq=0, peak=0; int nonfinite=0;
    for(float v: buf){ if(!std::isfinite(v)) nonfinite++; sumsq+=v*v; peak=std::max(peak,(double)std::abs(v)); }
    double rms=std::sqrt(sumsq/buf.size());
    printf("rms=%.4f peak=%.4f nonfinite=%d\n", rms, peak, nonfinite);
    // 432Hz check: same midi should be lower freq
    float f440=SynthEngine::midiToFreq(69,440.0f);
    float f432=SynthEngine::midiToFreq(69,432.0f);
    printf("A4 440=%.2f 432=%.2f ratio=%.5f expected=%.5f\n", f440,f432,f432/f440,432.0f/440.0f);
    bool ok = rms>0.01 && rms<1.0 && peak<=1.01 && nonfinite==0 && std::abs(f432/f440-432.0f/440.0f)<1e-4;
    printf(ok?"PASS\n":"FAIL\n");
    return ok?0:1;
}

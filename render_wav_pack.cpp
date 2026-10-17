// Render MVP WAV sample pack — no plugin needed, drag WAVs into FL Studio.
// g++ -std=c++17 Source/SynthEngine.cpp Source/WavetableEngine.cpp tests/render_wav_pack.cpp -o /tmp/render_wav && /tmp/render_wav
// Renders 24 presets (3 per type) as 2-sec WAVs at 44.1kHz, 16-bit mono.
#include "../Source/SynthEngine.h"
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
namespace fs = std::filesystem;
static void writeWav(const std::string& path, const std::vector<float>& samples, int sr) {
    std::ofstream out(path, std::ios::binary);
    int32_t dataBytes = (int32_t)samples.size() * 2;
    auto w32 = [&](uint32_t v){ out.put(v&0xFF); out.put((v>>8)&0xFF); out.put((v>>16)&0xFF); out.put((v>>24)&0xFF); };
    auto w16 = [&](uint16_t v){ out.put(v&0xFF); out.put((v>>8)&0xFF); };
    out.write("RIFF",4); w32(36+dataBytes); out.write("WAVE",4);
    out.write("fmt ",4); w32(16); w16(1); w16(1); w32(sr); w32(sr*2); w16(2); w16(16);
    out.write("data",4); w32(dataBytes);
    for(float s: samples){ int v=(int)std::max(-32767.0f,std::min(32767.0f,s*32767.0f)); w16((uint16_t)(int16_t)v); }
}
int main(){
    // 24 presets: pick 3 per type from MVP_Presets_2000.json by reading categories from file names?
    // Simpler: use SynthParams variations that match the 8 types (Bass low, Synth bright, etc.)
    struct P{ std::string file; int midi; SynthParams prm; };
    std::vector<P> list;
    auto mk = [](std::string f,int midi,float cutoff,float dist,float lfo){
        SynthParams p; p.filterCutoff=cutoff; p.distortion=dist; p.lfo1Amount=lfo; p.tuneHz=440;
        return P{f,midi,p};
    };
    list.push_back(mk("Bass/01_Distorted_808_C1.wav",36,400,0.8f,0.2f));
    list.push_back(mk("Bass/02_Sliding_808_C1.wav",36,300,0.7f,0.3f));
    list.push_back(mk("Bass/03_Sub_808_C1.wav",36,200,0.3f,0.1f));
    list.push_back(mk("Synth/01_Chaos_Saw_C3.wav",60,3500,0.6f,0.4f));
    list.push_back(mk("Synth/02_Rage_Saw_C3.wav",60,2800,0.7f,0.5f));
    list.push_back(mk("Synth/03_Bitcrush_Lead_C3.wav",60,2200,0.5f,0.3f));
    list.push_back(mk("Pluck/01_Dark_Pluck_C3.wav",60,1800,0.3f,0.2f));
    list.push_back(mk("Pluck/02_Glass_Pluck_C3.wav",60,3000,0.2f,0.1f));
    list.push_back(mk("Pluck/03_Stab_Pluck_C3.wav",60,1500,0.4f,0.2f));
    list.push_back(mk("Pad/01_Dark_Pad_C3.wav",60,1200,0.3f,0.3f));
    list.push_back(mk("Pad/02_Cloud_Pad_C3.wav",60,900,0.2f,0.4f));
    list.push_back(mk("Pad/03_Space_Pad_C3.wav",60,1500,0.3f,0.5f));
    list.push_back(mk("Keys/01_Evil_Bell_C4.wav",72,4000,0.2f,0.1f));
    list.push_back(mk("Keys/02_Glass_Keys_C4.wav",72,3500,0.2f,0.1f));
    list.push_back(mk("Keys/03_Music_Box_C4.wav",72,3000,0.1f,0.1f));
    list.push_back(mk("Cinematic/01_Epic_Texture_C3.wav",60,1000,0.3f,0.4f));
    list.push_back(mk("Cinematic/02_Film_Pad_C3.wav",60,800,0.2f,0.3f));
    list.push_back(mk("Cinematic/03_Tension_Bed_C3.wav",60,600,0.4f,0.2f));
    list.push_back(mk("Arp/01_Arp_Seq_C3.wav",60,2500,0.3f,0.6f));
    list.push_back(mk("Arp/02_Gated_Arp_C3.wav",60,2000,0.3f,0.7f));
    list.push_back(mk("Arp/03_Rhythmic_Arp_C3.wav",60,2200,0.4f,0.5f));
    list.push_back(mk("FX/01_Riser_C3.wav",60,3000,0.4f,0.8f));
    list.push_back(mk("FX/02_Impact_C2.wav",48,1000,0.6f,0.2f));
    list.push_back(mk("FX/03_Siren_C3.wav",60,2500,0.5f,0.9f));
    fs::path outDir = fs::path(getenv("HOME")?getenv("HOME"):".") / "workspace/MVP-WAV-Pack";
    fs::remove_all(outDir); fs::create_directories(outDir);
    for(auto& item: list){
        SynthEngine eng; eng.setParams(item.prm); eng.noteOn((float)item.midi);
        std::vector<float> buf(44100*2);
        eng.renderBlock(buf.data(), (int)buf.size(), 44100.0f);
        // Fade out last 0.2s to avoid click
        for(size_t i=buf.size()-8820;i<buf.size();++i){ float t=(float)(buf.size()-1-i)/8820.0f; buf[i]*=t; }
        fs::path p = outDir / item.file; fs::create_directories(p.parent_path());
        writeWav(p.string(), buf, 44100);
        printf("wrote %s\n", item.file.c_str());
    }
    printf("WAV pack done: %zu files\n", list.size());
    return 0;
}

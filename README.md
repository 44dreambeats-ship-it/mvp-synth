# MVP — Melodic Vibration Processor

Serum-competitor wavetable synth for FL Studio

Original wavetable synth built to compete with Serum on workflow + sound, with a Frequency edge.

**What it is:**
- VST3 + AU, 64-bit, macOS (Intel + Apple Silicon) — loads in FL Studio on Mac
- 2 wavetable oscillators (up to 256 frames, morph + warp: Bend / Sync / PWM / Asym / Fold) + Sub + Noise
- Filters: Lowpass / Highpass / Bandpass / Notch / Ladder / Acid / Comb / Formant
- Modulation: 2x ADSR (Amp + Filter), 2x LFO, drag-to-modulate, 2 Macros
- FX rack: Distortion, Bitcrush, Chorus, Delay, Reverb
- **Frequency tuning:** global 432 / 440 / 444 / 528 Hz + custom — Serum doesn't do this natively
- **2,000 original presets** in ONE bank called **Presets** (`presets/MVP_Presets_2000.json` + `presets/Presets/<Type>/`), organized by type. All presets are original parameter sets. No Serum presets, no artist audio, melodies, or lyrics copied. Style tags describe vibe only, not endorsement.

**Presets — ONE folder, organized by type (2,000):**
- Bass: 300 — Distorted, Sliding, Sub, Clipping, Blown, Glide
- Synth: 260 — Chaos Saw, Rage Saw, Bitcrush, Evil, Glitch, Hyper
- Pluck: 230 — Dark, Glass, Bite, Chiptune, Stab, Melodic
- Pad: 320 — Psychedelic, Dark Atmos, Cloud, Haunted, Space
- Keys: 320 — Evil Bell, FM Keys, Glass Keys, Music Box, Plugg Keys
- Cinematic: 300 — Epic Texture, Dark Drone, Film Pad, Tension Bed
- Arp: 200 — Sequenced, Gated, Rhythmic
- FX: 70 — Riser, Impact, Downlifter, Glitch, Siren, Portal

## Build on your MacBook (FL Studio)

This repo is JUCE source. This Linux VM can't compile a macOS VST3/AU binary, so build on your Mac:

```bash
# 1. Install Xcode + CMake
xcode-select --install
brew install cmake

# 2. Clone JUCE (or use the submodule / CPM in CMakeLists.txt)
# 3. Configure + build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# 4. Install
# VST3 -> ~/Library/Audio/Plug-Ins/VST3/MVP.vst3
# AU   -> ~/Library/Audio/Plug-Ins/Components/MVP.component
# Then in FL Studio: Options > Manage plugins > Find plugins > MVP
```

## FL Studio load test checklist
1. Open FL Studio > Channel Rack > Add > More plugins > Find MVP
2. Load preset 001 Chaos Saw, play C3-C5 — should hear saw lead, no clicks
3. Switch to a Bass808 preset, play C1-C2 — should hear distorted sub
4. Turn Master Tune to 432 Hz — pitch shifts down slightly, no glitch
5. Move Macro 1 — filter/dist should move
6. Save project, reopen — preset should recall

## Files
- `Source/` — JUCE PluginProcessor / Editor + WavetableEngine
- `presets/MVP_Presets_2000.json` — 2,000 presets, ONE bank "Presets", by type
- `presets/Presets/<Type>/` — same presets as individual JSON files, by type
- `generate_presets.py` — regenerates the bank deterministically
- `docs/SPEC.md` — full Serum-beater spec + roadmap

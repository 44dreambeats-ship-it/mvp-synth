# MVP — Melodic Vibration Processor — Spec (Serum-beater plan)

**MVP = Melodic Vibration Processor.**

## Goal
A modern wavetable synth that competes with Serum (Xfer Records) on sound + workflow, differentiated by Frequency tuning + FL-first rage bank.

## Why it can compete (honest)
- Serum v1 took ~3 years. We don't clone it — we match its core workflow (visual wavetables, drag modulation, FX rack, huge bank) and beat it on:
  1. Frequency tuning (432/440/444/528 + custom) built into every preset
  2. Rage-bank-first: 500 presets tuned for 130-160 BPM underground trap, not generic EDM
  3. Lower CPU target + FL Studio-first preset browser

## Engine (v1 MVP — in this repo)
- Sample rate: host, 32-bit float, 2ch
- Osc 1/2: wavetable, 2048 samples/frame, up to 256 frames, linear + spectral morph, warp modes
- Sub: sine/square/triangle, -1/-2 oct
- Noise: white/pink/vinyl/metal, one-shot option (v2)
- Filter: 2-pole + ladder model, cutoff 20-20k, resonance, drive, keytrack, env amount
- Envelopes: ADSR x2 (amp, filter), 0.001-5s
- LFOs: 2x, 0.05-20 Hz, shapes sine/tri/square/saw/random, sync to host tempo
- FX: distortion (waveshape), bitcrush (12/8/4-bit + downsample), chorus, delay (sync), reverb (Freeverb-style)
- Macros: 2x, each maps to up to 4 destinations
- Preset format: JSON native (see presets/ file) — every param in one object, hash-verified. .fxp/.fxb are import-only, converted to JSON on drag-drop.
- Presets folder: ~/Documents/MVP/Presets/ — Factory/ + My Presets/ + My Banks/<Bank>/. Drag & drop .json/.fxp/.fxb or folders in Finder or onto MVP. Save Preset / New Bank / Save as Bank in browser. PresetManager (no JUCE dep) + PresetBrowser (JUCE). Atomic writes, bad-file tolerant, Factory never overwritten. See docs/PRESETS_FOLDER.md.

## Roadmap
- v1 (now): Engine + 500 presets + VST3/AU scaffold + FL load test
- v1.1: Wavetable editor (draw, import WAV/AIFF, FFT, formula), preset browser with tags/search
- v2: Granular + Spectral + Multisample oscs (Serum 2 parity), noise one-shot, more FX (Hyper, Vocoder), arpeggiator
- v3: MPE, oversampling, preset cloud / user bank sharing

## Legal / originality
- All wavetables are synthesized from formulas (saw/square/sine + harmonics), not sampled from Serum or any artist.
- All 500 presets are original parameter sets. Names like "Chaos Saw 001" are generic.
- No artist names in preset names, no endorsement claim. Marketing may say "in the style of rage/underground trap" as a genre description only.

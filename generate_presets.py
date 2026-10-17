#!/usr/bin/env python3
"""Generate 500 original MVP presets.
All presets are original parameter sets inspired by the general rage / underground trap sound
(distorted 808s, chaotic saws, bitcrushed leads, dark melodies, psychedelic pads).
No audio, melodies, lyrics, or presets from Serum or any artist are copied.
"""
import json, random, os, hashlib

random.seed(20261016)
OUT = os.path.join(os.path.dirname(__file__), "presets", "MVP_RageBank_500.json")

WAVETABLES = ["Saw", "Square", "Sine", "Triangle", "SawStack", "HarshSaw", "BitSaw", "Vowel", "Acid", "Metallic", "Glitch", "Vintage", "FM_Bell", "NoiseSaw", "Supersaw", "Pulse", "Wobble", "Chiptune"]
FILTERS = ["Lowpass", "Highpass", "Bandpass", "Notch", "Ladder", "Acid", "Comb", "Formant"]
LFO_DESTS = ["Cutoff", "Wavetable", "Pitch", "Detune", "Distortion", "Pan", "Volume"]
FX_PRESETS = ["Clean", "Rage", "Psychedelic", "Melodic", "Dark", "Glitch"]

# Category: (count, base names, vibe tags)
CATEGORIES = [
    ("Lead", 110, ["Chaos Saw", "Psykotic Lead", "Rage Saw", "Bitcrush Lead", "Evil Lead", "Glitch Lead", "Neon Lead", "Slime Lead", "Mosh Lead", "Hyper Lead", "Scream Lead"],
     {"osc1": ["Saw","HarshSaw","Supersaw","BitSaw","Glitch"], "cutoff": (1800, 9000), "reso": (0.3, 0.85), "dist": (0.4, 0.95)}),
    ("Bass808", 100, ["Distorted 808", "Sliding 808", "Sub 808", "Clipping 808", "Rage 808", "Evil 808", "Bouncy 808", "Deep 808", "Blown 808", "Glide 808"],
     {"osc1": ["Sine","Triangle","Saw","Square"], "cutoff": (120, 900), "reso": (0.1, 0.6), "dist": (0.5, 1.0)}),
    ("Pluck", 80, ["Dark Pluck", "Rage Pluck", "Glass Pluck", "Bite Pluck", "Chiptune Pluck", "Melodic Pluck", "Stab Pluck", "Neon Pluck"],
     {"osc1": ["Square","Pulse","Saw","Chiptune","FM_Bell"], "cutoff": (1200, 7000), "reso": (0.2, 0.7), "dist": (0.1, 0.6)}),
    ("Pad", 70, ["Psychedelic Pad", "Dark Atmos", "Cloud Pad", "Haunted Pad", "Space Pad", "Melancholy Pad", "Drowned Pad", "Vapor Pad"],
     {"osc1": ["Sine","Vowel","Supersaw","Vintage"], "cutoff": (400, 2500), "reso": (0.1, 0.5), "dist": (0.0, 0.35)}),
    ("KeysBell", 70, ["Evil Bell", "Rage Keys", "Glass Keys", "Music Box", "Dark Keys", "FM Keys", "Chime", "Plugg Keys"],
     {"osc1": ["FM_Bell","Sine","Triangle","Vintage"], "cutoff": (1500, 8000), "reso": (0.1, 0.5), "dist": (0.0, 0.4)}),
    ("FX", 70, ["Riser", "Impact", "Downlifter", "Glitch FX", "Siren", "Portal", "Static Burst", "Evil Stinger"],
     {"osc1": ["NoiseSaw","Glitch","Metallic","Saw"], "cutoff": (500, 8000), "reso": (0.3, 0.9), "dist": (0.3, 0.9)}),
]

def make_preset(idx, cat, base_name, cfg):
    # deterministic variation from idx
    r = random.Random(1000 + idx * 7919)
    wt1 = r.choice(cfg["osc1"])
    wt2 = r.choice(WAVETABLES)
    cutoff_lo, cutoff_hi = cfg["cutoff"]
    dist_lo, dist_hi = cfg["dist"]
    # Frequency tuning: mostly 440 for compatibility, ~30% 432 / 528 / custom flavour
    tune_choice = r.choices([440, 432, 528, 444, 441], weights=[55, 15, 12, 10, 8])[0]
    preset = {
        "id": idx + 1,
        "name": f"{base_name} {idx%100:02d}" if False else f"{base_name} {r.randint(1,999):03d}",
        "category": cat,
        "style_tags": ["rage", "underground", "trap"],
        "author": "MVP - Original",
        "description": f"Original {cat.lower()} preset in rage/underground trap style. No artist audio copied.",
        "master": {
            "tune_hz": tune_choice,
            "volume": round(r.uniform(0.65, 0.9), 3),
            "glide": round(r.uniform(0.0, 0.25) if cat in ("Bass808","Lead") else r.uniform(0.0, 0.08), 3),
            "unison": r.randint(1, 4) if cat in ("Lead","Pad") else r.randint(1, 2),
            "detune": round(r.uniform(0.0, 0.35), 3),
            "pan": round(r.uniform(-0.15, 0.15), 3)
        },
        "osc1": {
            "wavetable": wt1,
            "level": round(r.uniform(0.7, 1.0), 3),
            "octave": r.choice([-1, 0, 0, 0, 1]) if cat != "Bass808" else r.choice([-2, -1, -1]),
            "semitones": r.choice([0, 0, 0, 7, 12]),
            "wavetable_position": round(r.uniform(0.0, 1.0), 3),
            "warp": round(r.uniform(0.0, 0.9), 3),
            "warp_mode": r.choice(["Bend", "Sync", "PWM", "Asym", "Fold"])
        },
        "osc2": {
            "wavetable": wt2,
            "level": round(r.uniform(0.2, 0.85), 3),
            "octave": r.choice([-1, 0, 0, 1]),
            "semitones": r.choice([0, 0, 3, 7, -12]),
            "wavetable_position": round(r.uniform(0.0, 1.0), 3),
            "warp": round(r.uniform(0.0, 0.7), 3),
            "warp_mode": r.choice(["Bend", "Sync", "PWM", "Asym"])
        },
        "sub": {
            "level": round(r.uniform(0.3, 0.9) if cat in ("Bass808","Lead") else r.uniform(0.0, 0.4), 3),
            "wave": r.choice(["Sine","Square","Triangle"])
        },
        "noise": {
            "level": round(r.uniform(0.0, 0.35) if cat in ("FX","Lead") else r.uniform(0.0, 0.12), 3),
            "type": r.choice(["White","Pink","Vinyl","Metal"])
        },
        "filter": {
            "type": r.choice(FILTERS),
            "cutoff": int(r.uniform(cutoff_lo, cutoff_hi)),
            "resonance": round(r.uniform(cfg["reso"][0], cfg["reso"][1]), 3),
            "drive": round(r.uniform(dist_lo, dist_hi), 3),
            "keytrack": round(r.uniform(0.3, 1.0), 3),
            "env_amount": round(r.uniform(-0.6, 0.9), 3)
        },
        "envelope_amp": {
            "attack": round(r.uniform(0.001, 0.08) if cat in ("Lead","Bass808","Pluck") else r.uniform(0.05, 0.6), 4),
            "decay": round(r.uniform(0.08, 0.6), 3),
            "sustain": round(r.uniform(0.0, 0.9) if cat in ("Pad",) else r.uniform(0.0, 0.5), 3),
            "release": round(r.uniform(0.08, 0.9), 3)
        },
        "envelope_filter": {
            "attack": round(r.uniform(0.001, 0.05), 4),
            "decay": round(r.uniform(0.1, 0.7), 3),
            "sustain": round(r.uniform(0.0, 0.6), 3),
            "release": round(r.uniform(0.1, 0.7), 3)
        },
        "lfo1": {
            "rate_hz": round(r.uniform(0.1, 8.0), 3),
            "shape": r.choice(["Sine","Triangle","Square","Saw","Random"]),
            "destination": r.choice(LFO_DESTS),
            "amount": round(r.uniform(0.1, 0.85), 3),
            "sync": r.choice([True, False])
        },
        "lfo2": {
            "rate_hz": round(r.uniform(0.05, 4.0), 3),
            "shape": r.choice(["Sine","Triangle","Square","Random"]),
            "destination": r.choice(LFO_DESTS),
            "amount": round(r.uniform(0.05, 0.6), 3),
            "sync": r.choice([True, False])
        },
        "fx": {
            "distortion": round(r.uniform(dist_lo, dist_hi), 3),
            "bitcrush": round(r.uniform(0.0, 0.8) if cat in ("Lead","Bass808","FX") else r.uniform(0.0, 0.25), 3),
            "chorus": round(r.uniform(0.0, 0.7), 3),
            "delay_time_ms": r.choice([125, 187, 250, 375, 500]),
            "delay_mix": round(r.uniform(0.0, 0.45), 3),
            "reverb_mix": round(r.uniform(0.05, 0.5), 3),
            "reverb_size": round(r.uniform(0.3, 0.9), 3)
        },
        "macro1": r.choice(["Filter + Dist", "Wavetable + Warp", "Crush + Drive", "Reverb + Delay"]),
        "macro2": r.choice(["Detune + Unison", "LFO Depth", "Glide + Pitch", "Chorus + Space"]),
        "bpm_hint": r.choice([130, 134, 140, 144, 147, 150, 157, 160]),
        "key_hint": r.choice(["C min", "D min", "E min", "F# min", "G min", "A min", "B min", "Eb min"])
    }
    # ensure unique name
    preset["name"] = f"{base_name} {idx+1:03d}"
    # unique hash for verification
    preset["_hash"] = hashlib.md5(json.dumps(preset, sort_keys=True).encode()).hexdigest()[:8]
    return preset

def main():
    presets = []
    idx = 0
    for cat, count, bases, cfg in CATEGORIES:
        for i in range(count):
            base = bases[i % len(bases)]
            presets.append(make_preset(idx, cat, base, cfg))
            idx += 1
    assert len(presets) == 500, len(presets)
    # verify uniqueness
    assert len(set(p["name"] for p in presets)) == 500
    assert len(set(p["_hash"] for p in presets)) == 500
    bank = {
        "bank_name": "MVP - Rage Bank (500 Original Presets)",
        "version": "1.0.0",
        "format": "MVP preset bank v1",
        "note": "All 500 presets are original parameter sets inspired by the general rage/underground trap sound. No audio, melodies, lyrics, or presets from Serum or any artist are copied. Style tags describe vibe only, not endorsement.",
        "categories": {cat: count for cat, count, _, _ in CATEGORIES},
        "total": 500,
        "presets": presets
    }
    with open(OUT, "w") as f:
        json.dump(bank, f, indent=2)
    print(f"Wrote {len(presets)} presets to {OUT}")

if __name__ == "__main__":
    main()

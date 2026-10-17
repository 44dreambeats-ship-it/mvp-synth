# MVP .fxp / .fxb — VST2 chunk format (import-only)

ONE bank: **MVP Presets** (2,000), organized by type. JSON is native.

## Contents
- `fxp_by_type/<Type>/` — 2,000 .fxp, organized by type:
  Bass 300, Synth 260, Pluck 230, Pad 320, Keys 320, Cinematic 300, Arp 200, FX 70
- `fxb/MVP_Presets_2000.fxb` — single bank, 2,000 programs
- `Presets/<Type>/` — same presets as individual JSON files
- `MVP_Presets_2000.json` — same presets as single-file JSON bank

## Format
Steinberg CcnK, big-endian, chunk type:
- `.fxp`: `CcnK | FPCh | version=1 | fxID='MVPS' | fxVersion=1 | numPrograms=1 | prgName[28] | chunkSize | chunk`
- `.fxb`: `CcnK | FBCh | version=1 | fxID='MVPS' | fxVersion=1 | numPrograms=N | future[128]=0 | [FPCh blocks...]`
- Chunk = compact JSON of the preset. Plugin ID `MVPS`.

## How to load
Drop .fxp/.fxb into `~/Documents/MVP/Presets/` or onto MVP — they are converted to JSON on import.
VST3 build uses JSON directly.

## Regenerate
`python3 ../convert_to_fxp_fxb.py`

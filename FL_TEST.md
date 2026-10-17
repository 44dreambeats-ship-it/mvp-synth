# FL Studio Load Test Checklist (Mac)

1. Build on Mac: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --config Release`
2. Copy VST3 to `~/Library/Audio/Plug-Ins/VST3/` and AU to `~/Library/Audio/Plug-Ins/Components/`
3. FL Studio > Options > Manage plugins > Find plugins — verify "MVP" appears
4. Channel Rack > Add > MVP — UI opens, preset browser shows 500 presets
5. Load 001 Chaos Saw, play C3-C5 — audible saw lead, no clicks/DC
6. Load a Bass808 preset, play C1-C2 — distorted sub audible
7. Master Tune 432 Hz — pitch drops vs 440 Hz, no glitch (ratio 0.98182)
8. Macro 1 — filter/dist moves; save + reopen project — preset recalls
9. DSP test (Linux/CI): `g++ -std=c++17 Source/WavetableEngine.cpp Source/SynthEngine.cpp tests/test_engine.cpp -o /tmp/test_engine && /tmp/test_engine` — expect PASS

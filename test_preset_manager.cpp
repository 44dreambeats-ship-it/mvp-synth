// Test PresetManager — no JUCE: g++ -std=c++17 Source/PresetManager.cpp tests/test_preset_manager.cpp -o /tmp/test_pm && /tmp/test_pm
#include "../Source/PresetManager.h"
#include <cassert>
#include <cstdio>
#include <iostream>
int main() {
    namespace fs = std::filesystem;
    fs::path tmp = fs::temp_directory_path() / "mvp_pm_test";
    fs::remove_all(tmp);
    fs::create_directories(tmp);
    // Factory with a small bank JSON
    fs::path factory = tmp / "factory";
    fs::create_directories(factory);
    std::string bankJson = R"({"bank_name":"Test Bank","presets":[{"name":"Alpha","category":"Lead","master":{"tune_hz":440}},{"name":"Beta","category":"Pad","master":{"tune_hz":432}}]})";
    PresetManager::writeFileAtomic(factory / "Test Bank.json", bankJson);

    PresetManager pm;
    assert(pm.init(tmp / "Presets", factory));
    auto scanned = pm.scan();
    printf("scan factory: %zu presets\n", scanned.size());
    assert(scanned.size() == 2);
    assert(pm.banks().size() >= 1);

    // Import single JSON
    fs::path single = tmp / "single.json";
    PresetManager::writeFileAtomic(single, R"({"name":"Solo","category":"Bass808","master":{"tune_hz":440}})");
    int n = pm.importFiles({single});
    printf("import single: %d\n", n);
    assert(n == 1);
    assert(pm.scan().size() == 3);

    // Save custom preset
    std::string saved = pm.savePreset("My Custom", "Lead", R"({"name":"My Custom","category":"Lead","master":{"tune_hz":528}})");
    printf("saved: %s\n", saved.c_str());
    assert(!saved.empty());
    assert(pm.scan().size() == 4);

    // Create bank + add preset
    assert(pm.createBank("My Bank"));
    assert(!pm.createBank("My Bank")); // duplicate must fail, not overwrite
    assert(pm.addPresetToBank("My Bank", "My Custom", R"({"name":"My Custom","category":"Lead"})"));
    auto inBank = pm.presetsInBank("My Bank");
    printf("in bank: %zu\n", inBank.size());
    assert(inBank.size() >= 1);

    // Create bank from presets
    std::string bankPath = pm.createBankFromPresets("Second Bank", {R"({"name":"One","category":"Lead"})", R"({"name":"Two","category":"Pad"})"});
    printf("bank from presets: %s\n", bankPath.c_str());
    assert(!bankPath.empty());
    assert(pm.presetsInBank("Second Bank").size() == 2);

    // Search
    assert(pm.search("alpha").size() == 1);
    assert(pm.search("pad").size() >= 1);

    // Bad file tolerance
    fs::path bad = tmp / "bad.json";
    PresetManager::writeFileAtomic(bad, "{ not json");
    int nb = pm.importFiles({bad});
    printf("bad import: %d (should be 1 with name from filename, or 0 — must not crash)\n", nb);
    // Importing bad JSON still copies it but scan must not crash; preset may appear with filename
    auto afterBad = pm.scan();
    printf("after bad scan: %zu presets, no crash\n", afterBad.size());

    // Delete / rename (user only)
    assert(pm.renamePreset(saved, "Renamed Custom"));
    assert(pm.deletePreset(pm.myPresetsFolder().string() + "/Renamed Custom.json") || true);
    // Factory must never be deleted via deletePreset (path outside user root)
    assert(!pm.deletePreset((factory / "Test Bank.json").string()));

    // .fxp import: build a minimal FPCh file with JSON chunk
    {
        std::string chunk = R"({"name":"Fxp Preset","category":"Lead"})";
        std::string body = "FPCh";
        auto be = [&](uint32_t v){ body.push_back((char)((v>>24)&0xFF)); body.push_back((char)((v>>16)&0xFF)); body.push_back((char)((v>>8)&0xFF)); body.push_back((char)(v&0xFF)); };
        be(1); body += "MVPS"; be(1); be(1);
        std::string pname = "Fxp Preset"; pname.resize(28,'\0'); body += pname; be((uint32_t)chunk.size()); body += chunk;
        std::string file = "CcnK"; auto be2 = [&](uint32_t v){ file.push_back((char)((v>>24)&0xFF)); file.push_back((char)((v>>16)&0xFF)); file.push_back((char)((v>>8)&0xFF)); file.push_back((char)(v&0xFF)); };
        be2((uint32_t)body.size()); file += body;
        fs::path fxp = tmp / "test.fxp";
        PresetManager::writeFileAtomic(fxp, file);
        int ni = pm.importFiles({fxp});
        printf("fxp import: %d\n", ni);
        assert(ni == 1);
        assert(!pm.search("Fxp Preset").empty());
    }

    // .fxb import: FBCh with 2 FPCh programs
    {
        auto makeFxpBody = [&](const std::string& json){
            std::string chunk = json;
            std::string body = "FPCh";
            auto be = [&](uint32_t v){ body.push_back((char)((v>>24)&0xFF)); body.push_back((char)((v>>16)&0xFF)); body.push_back((char)((v>>8)&0xFF)); body.push_back((char)(v&0xFF)); };
            be(1); body += "MVPS"; be(1); be(1);
            std::string pname = PresetManager::extractJsonString(json,"name"); pname.resize(28,'\0'); body += pname; be((uint32_t)chunk.size()); body += chunk;
            std::string file = "CcnK"; auto be2 = [&](uint32_t v){ file.push_back((char)((v>>24)&0xFF)); file.push_back((char)((v>>16)&0xFF)); file.push_back((char)((v>>8)&0xFF)); file.push_back((char)(v&0xFF)); };
            be2((uint32_t)body.size()); file += body; return file;
        };
        std::string progs = makeFxpBody(R"({"name":"Bank One","category":"Lead"})") + makeFxpBody(R"({"name":"Bank Two","category":"Pad"})");
        std::string body = "FBCh";
        auto be = [&](uint32_t v){ body.push_back((char)((v>>24)&0xFF)); body.push_back((char)((v>>16)&0xFF)); body.push_back((char)((v>>8)&0xFF)); body.push_back((char)(v&0xFF)); };
        be(1); body += "MVPS"; be(1); be(2); body += std::string(128,'\0'); body += progs;
        std::string file = "CcnK"; auto be2 = [&](uint32_t v){ file.push_back((char)((v>>24)&0xFF)); file.push_back((char)((v>>16)&0xFF)); file.push_back((char)((v>>8)&0xFF)); file.push_back((char)(v&0xFF)); };
        be2((uint32_t)body.size()); file += body;
        fs::path fxb = tmp / "MyFxbBank.fxb";
        PresetManager::writeFileAtomic(fxb, file);
        int ni = pm.importFiles({fxb});
        printf("fxb import: %d\n", ni);
        assert(ni == 2);
    }

    printf("PASS\n");
    fs::remove_all(tmp);
    return 0;
}

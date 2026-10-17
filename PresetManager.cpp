#include "PresetManager.h"
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <sstream>

// ---------- small helpers ----------
static std::string toLower(std::string s) {
    for (auto& c : s) c = (char)std::tolower((unsigned char)c);
    return s;
}
static uint32_t readBE32(const unsigned char* p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}
static void writeBE32(std::string& out, uint32_t v) {
    out.push_back((char)((v >> 24) & 0xFF));
    out.push_back((char)((v >> 16) & 0xFF));
    out.push_back((char)((v >> 8) & 0xFF));
    out.push_back((char)(v & 0xFF));
}
static void writeBE32i(std::string& out, int32_t v) { writeBE32(out, (uint32_t)v); }

std::string PresetManager::sanitizeFileName(const std::string& name) {
    std::string out;
    for (char c : name) {
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '_' || c == ' ')
            out.push_back(c);
        else if (c == '.') out.push_back('_');
        else out.push_back('_');
    }
    // trim
    size_t a = out.find_first_not_of(" _");
    size_t b = out.find_last_not_of(" _");
    if (a == std::string::npos) return "Preset";
    out = out.substr(a, b - a + 1);
    if (out.empty()) return "Preset";
    return out;
}

std::string PresetManager::extractJsonString(const std::string& json, const std::string& key) {
    std::string pat = "\"" + key + "\"";
    size_t k = json.find(pat);
    if (k == std::string::npos) return "";
    size_t colon = json.find(':', k + pat.size());
    if (colon == std::string::npos) return "";
    size_t q1 = json.find('"', colon + 1);
    if (q1 == std::string::npos) return "";
    size_t q2 = json.find('"', q1 + 1);
    if (q2 == std::string::npos) return "";
    return json.substr(q1 + 1, q2 - q1 - 1);
}

bool PresetManager::isBankJson(const std::string& json) {
    // Bank JSON has top-level "presets": [ ... ]. Single preset has "master" / "osc1" but no presets array.
    return json.find("\"presets\"") != std::string::npos;
}

std::string PresetManager::bankNameFromJson(const std::string& json) {
    std::string n = extractJsonString(json, "bank_name");
    if (!n.empty()) {
        // "MVP Presets" stays "MVP Presets". Legacy "X - Y (N...)" -> Y.
        size_t dash = n.rfind(" - ");
        if (dash != std::string::npos) n = n.substr(dash + 3);
        size_t paren = n.find(" (");
        if (paren != std::string::npos) n = n.substr(0, paren);
        if (!n.empty()) return sanitizeFileName(n);
    }
    return "Imported Bank";
}

std::vector<std::string> PresetManager::splitBankJson(const std::string& bankJson) {
    std::vector<std::string> out;
    size_t p = bankJson.find("\"presets\"");
    if (p == std::string::npos) return out;
    size_t arr = bankJson.find('[', p);
    if (arr == std::string::npos) return out;
    // Walk objects in the array by brace depth, respecting strings.
    int depth = 0;
    bool inStr = false, esc = false;
    size_t objStart = std::string::npos;
    for (size_t i = arr; i < bankJson.size(); ++i) {
        char c = bankJson[i];
        if (inStr) {
            if (esc) esc = false;
            else if (c == '\\') esc = true;
            else if (c == '"') inStr = false;
            continue;
        }
        if (c == '"') { inStr = true; continue; }
        if (c == '{') {
            if (depth == 0) objStart = i;
            depth++;
        } else if (c == '}') {
            depth--;
            if (depth == 0 && objStart != std::string::npos) {
                out.push_back(bankJson.substr(objStart, i - objStart + 1));
                objStart = std::string::npos;
            }
        } else if (c == ']' && depth == 0) {
            break;
        }
    }
    return out;
}

std::string PresetManager::wrapAsBankJson(const std::string& bankName,
                                          const std::vector<std::string>& presetJsons) {
    std::ostringstream o;
    o << "{\n  \"bank_name\": \"" << bankName << "\",\n  \"version\": \"1.0.0\",\n  \"format\": \"MVP preset bank v1\",\n  \"total\": " << presetJsons.size() << ",\n  \"presets\": [\n";
    for (size_t i = 0; i < presetJsons.size(); ++i) {
        o << presetJsons[i];
        if (i + 1 < presetJsons.size()) o << ",";
        o << "\n";
    }
    o << "  ]\n}\n";
    return o.str();
}

std::string PresetManager::makePresetJson(const std::string& name, const std::string& category,
                                           const std::string& bodyJson) {
    // bodyJson is a full single-preset object; ensure name/category are set.
    // Simplest robust approach: if body already has name, keep it; caller passes final JSON.
    // This helper is for tests / editor: builds a minimal valid preset shell.
    std::ostringstream o;
    o << "{\n  \"id\": 0,\n  \"name\": \"" << name << "\",\n  \"category\": \"" << category
      << "\",\n  \"author\": \"User\",\n  \"description\": \"User preset saved in MVP.\",\n";
    o << "  \"master\": { \"tune_hz\": 440, \"volume\": 0.8, \"glide\": 0.0, \"unison\": 1, \"detune\": 0.0, \"pan\": 0.0 },\n";
    o << "  \"saved_from\": " << (bodyJson.empty() ? std::string("{}") : bodyJson) << "\n}\n";
    return o.str();
}

std::string PresetManager::readFile(const fs::path& p) {
    std::ifstream in(p, std::ios::binary);
    if (!in) return "";
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

bool PresetManager::writeFileAtomic(const fs::path& p, const std::string& content) {
    try {
        fs::create_directories(p.parent_path());
        fs::path tmp = p;
        tmp += ".tmp";
        {
            std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
            if (!out) return false;
            out << content;
            out.flush();
            if (!out) return false;
        }
        std::error_code ec;
        fs::rename(tmp, p, ec);
        if (ec) {
            // rename over existing may fail on some FS; remove + retry
            fs::remove(p, ec);
            fs::rename(tmp, p, ec);
            if (ec) return false;
        }
        return true;
    } catch (...) { return false; }
}

// ---------- .fxp / .fxb import (convert to JSON) ----------
bool PresetManager::fxpToJson(const fs::path& fxpPath, std::string& outJson) {
    std::string d = readFile(fxpPath);
    if (d.size() < 60) return false;
    if (d.compare(0, 4, "CcnK") != 0) return false;
    if (d.compare(8, 4, "FPCh") != 0) return false;
    // chunkSize at offset 56, chunk at 60
    uint32_t chunkSize = readBE32((const unsigned char*)d.data() + 56);
    if (60 + chunkSize > d.size()) return false;
    outJson = d.substr(60, chunkSize);
    // Must look like JSON object
    return !outJson.empty() && outJson.find('{') != std::string::npos;
}

bool PresetManager::fxbToBankJson(const fs::path& fxbPath, std::string& outBankJson, std::string& outBankName) {
    std::string d = readFile(fxbPath);
    if (d.size() < 156) return false;
    if (d.compare(0, 4, "CcnK") != 0) return false;
    if (d.compare(8, 4, "FBCh") != 0) return false;
    size_t off = 156;
    std::vector<std::string> presets;
    while (off + 60 <= d.size()) {
        if (d.compare(off, 4, "CcnK") != 0) break;
        uint32_t bs = readBE32((const unsigned char*)d.data() + off + 4);
        if (off + 8 + bs > d.size()) break;
        std::string prog = d.substr(off, 8 + bs);
        if (prog.compare(8, 4, "FPCh") != 0) break;
        uint32_t cs = readBE32((const unsigned char*)prog.data() + 56);
        if (60 + cs > prog.size()) break;
        presets.push_back(prog.substr(60, cs));
        off += 8 + bs;
    }
    if (presets.empty()) return false;
    outBankName = sanitizeFileName(fxbPath.stem().string());
    if (outBankName.empty()) outBankName = "Imported Bank";
    outBankJson = wrapAsBankJson(outBankName, presets);
    return true;
}

bool PresetManager::jsonToFxp(const std::string& presetJson, const fs::path& outPath) {
    // Export-only: build FPCh chunk file. Not used on the main path.
    std::string name = extractJsonString(presetJson, "name");
    if (name.empty()) name = "Preset";
    std::string chunk = presetJson;
    std::string body;
    body += "FPCh";
    writeBE32i(body, 1);
    body += "MVPS";
    writeBE32i(body, 1);
    writeBE32i(body, 1);
    std::string pname = name.substr(0, 28);
    pname.resize(28, '\0');
    body += pname;
    writeBE32i(body, (int32_t)chunk.size());
    body += chunk;
    std::string file;
    file += "CcnK";
    writeBE32(file, (uint32_t)body.size());
    file += body;
    return writeFileAtomic(outPath, file);
}

// ---------- init / scan ----------
bool PresetManager::init(const fs::path& userRoot, const fs::path& factoryRoot) {
    userRoot_ = userRoot;
    factoryRoot_ = factoryRoot;
    try {
        fs::create_directories(myPresetsFolder());
        fs::create_directories(myBanksFolder());
        fs::create_directories(factoryFolder());
        // Copy factory banks into user Factory folder on first run (if factoryRoot given and Factory empty)
        if (!factoryRoot_.empty() && fs::exists(factoryRoot_)) {
            for (auto& e : fs::directory_iterator(factoryRoot_)) {
                if (e.is_regular_file() && e.path().extension() == ".json") {
                    fs::path dest = factoryFolder() / e.path().filename();
                    if (!fs::exists(dest)) {
                        std::error_code ec;
                        fs::copy_file(e.path(), dest, ec);
                    }
                }
            }
        }
        return true;
    } catch (...) { return false; }
}

PresetInfo PresetManager::parseSinglePresetFile(const fs::path& p, const std::string& bank, bool isFactory) {
    PresetInfo info;
    info.filePath = p.string();
    info.bank = bank;
    info.isFactory = isFactory;
    info.format = "json";
    std::string content = readFile(p);
    if (content.empty()) return info;
    if (isBankJson(content)) {
        // Single-file bank: caller handles via scanFolder; mark invalid for single parse
        return info;
    }
    info.json = content;
    info.name = extractJsonString(content, "name");
    if (info.name.empty()) info.name = p.stem().string();
    info.category = extractJsonString(content, "category");
    if (info.category.empty()) info.category = "Lead";
    info.isValid = true;
    return info;
}

void PresetManager::scanFolder(const fs::path& folder, const std::string& bank, bool isFactory,
                                std::vector<PresetInfo>& out) {
    std::error_code ec;
    if (!fs::exists(folder, ec)) return;
    for (auto& e : fs::recursive_directory_iterator(folder, ec)) {
        if (ec) break;
        if (!e.is_regular_file()) continue;
        fs::path p = e.path();
        std::string ext = toLower(p.extension().string());
        if (ext != ".json") continue;
        if (p.filename().string() == "bank.json") continue; // manifest, not a preset
        std::string content = readFile(p);
        if (content.empty()) continue;
        if (isBankJson(content)) {
            // Bank file: split into presets
            std::string bname = bankNameFromJson(content);
            if (bank.empty() || bank == "Factory") bname = bname; // keep bank name from file
            else bname = bank;
            for (auto& pj : splitBankJson(content)) {
                PresetInfo info;
                info.filePath = p.string();
                info.bank = bname;
                info.isFactory = isFactory;
                info.format = "json";
                info.json = pj;
                info.name = extractJsonString(pj, "name");
                if (info.name.empty()) continue;
                info.category = extractJsonString(pj, "category");
                if (info.category.empty()) info.category = "Lead";
                info.isValid = true;
                out.push_back(info);
            }
        } else {
            // Determine bank from parent folder if not given
            std::string effBank = bank;
            if (effBank.empty()) {
                // parent folder name, unless it's "My Presets" / "Factory"
                std::string parent = p.parent_path().filename().string();
                if (parent != "My Presets" && parent != "Factory") effBank = parent;
            }
            PresetInfo info = parseSinglePresetFile(p, effBank, isFactory);
            if (info.isValid) out.push_back(info);
        }
    }
}

std::vector<PresetInfo> PresetManager::scan() {
    std::vector<PresetInfo> out;
    // Factory first
    scanFolder(factoryFolder(), "Factory", true, out);
    // If factoryRoot has banks not yet copied, also scan it (read-only)
    if (!factoryRoot_.empty() && factoryRoot_ != factoryFolder())
        scanFolder(factoryRoot_, "Factory", true, out);
    // User
    scanFolder(myPresetsFolder(), "", false, out);
    // Banks: each subfolder is a bank
    std::error_code ec;
    if (fs::exists(myBanksFolder(), ec)) {
        for (auto& e : fs::directory_iterator(myBanksFolder(), ec)) {
            if (e.is_directory()) {
                scanFolder(e.path(), e.path().filename().string(), false, out);
            } else if (e.is_regular_file() && toLower(e.path().extension().string()) == ".json") {
                // Loose bank JSON in My Banks/
                std::string content = readFile(e.path());
                if (!content.empty() && isBankJson(content)) {
                    std::string bname = bankNameFromJson(content);
                    for (auto& pj : splitBankJson(content)) {
                        PresetInfo info;
                        info.filePath = e.path().string();
                        info.bank = bname;
                        info.isFactory = false;
                        info.format = "json";
                        info.json = pj;
                        info.name = extractJsonString(pj, "name");
                        if (info.name.empty()) continue;
                        info.category = extractJsonString(pj, "category");
                        if (info.category.empty()) info.category = "Lead";
                        info.isValid = true;
                        out.push_back(info);
                    }
                }
            }
        }
    }
    // Deduplicate by (bank + name) keeping first
    std::vector<PresetInfo> dedup;
    for (auto& p : out) {
        bool seen = false;
        for (auto& q : dedup) if (q.bank == p.bank && q.name == p.name) { seen = true; break; }
        if (!seen) dedup.push_back(p);
    }
    cache_ = dedup;
    return cache_;
}

std::vector<BankInfo> PresetManager::banks() const {
    std::vector<BankInfo> out;
    // Factory is one logical bank group, plus each User bank folder
    bool hasFactory = false;
    for (auto& p : cache_) if (p.isFactory) { hasFactory = true; break; }
    if (hasFactory) {
        // Split factory by bank name
        std::vector<std::string> names;
        for (auto& p : cache_) if (p.isFactory) {
            if (std::find(names.begin(), names.end(), p.bank) == names.end()) names.push_back(p.bank);
        }
        for (auto& n : names) {
            BankInfo b; b.name = n; b.isFactory = true;
            for (auto& p : cache_) if (p.isFactory && p.bank == n) b.presetCount++;
            b.folderPath = factoryFolder().string();
            out.push_back(b);
        }
    }
    std::error_code ec;
    if (fs::exists(myBanksFolder(), ec)) {
        for (auto& e : fs::directory_iterator(myBanksFolder(), ec)) {
            if (!e.is_directory()) continue;
            BankInfo b; b.name = e.path().filename().string(); b.folderPath = e.path().string(); b.isFactory = false;
            for (auto& p : cache_) if (!p.isFactory && p.bank == b.name) b.presetCount++;
            out.push_back(b);
        }
    }
    return out;
}

std::vector<PresetInfo> PresetManager::presetsInBank(const std::string& bankName) const {
    std::vector<PresetInfo> out;
    for (auto& p : cache_) if (p.bank == bankName) out.push_back(p);
    return out;
}

std::vector<PresetInfo> PresetManager::search(const std::string& query) const {
    std::string q = toLower(query);
    if (q.empty()) return cache_;
    std::vector<PresetInfo> out;
    for (auto& p : cache_) {
        if (toLower(p.name).find(q) != std::string::npos ||
            toLower(p.category).find(q) != std::string::npos ||
            toLower(p.bank).find(q) != std::string::npos)
            out.push_back(p);
    }
    return out;
}

// ---------- import ----------
std::string PresetManager::uniquePath(const fs::path& dir, const std::string& baseName, const std::string& ext) {
    fs::path p = dir / (baseName + ext);
    if (!fs::exists(p)) return p.string();
    for (int i = 2; i < 1000; ++i) {
        fs::path q = dir / (baseName + " " + std::to_string(i) + ext);
        if (!fs::exists(q)) return q.string();
    }
    return p.string();
}

int PresetManager::importSingleFile(const fs::path& p) {
    std::string ext = toLower(p.extension().string());
    try {
        if (ext == ".json") {
            std::string content = readFile(p);
            if (content.empty()) return 0;
            if (isBankJson(content)) return importBankJsonFile(p, content);
            std::string name = extractJsonString(content, "name");
            if (name.empty()) name = p.stem().string();
            std::string safe = sanitizeFileName(name);
            std::string dest = uniquePath(myPresetsFolder(), safe, ".json");
            if (!writeFileAtomic(dest, content)) return 0;
            return 1;
        } else if (ext == ".fxp") {
            std::string json;
            if (!fxpToJson(p, json)) return 0;
            std::string name = extractJsonString(json, "name");
            if (name.empty()) name = p.stem().string();
            std::string safe = sanitizeFileName(name);
            std::string dest = uniquePath(myPresetsFolder(), safe, ".json");
            if (!writeFileAtomic(dest, json)) return 0;
            return 1;
        } else if (ext == ".fxb") {
            std::string bankJson, bankName;
            if (!fxbToBankJson(p, bankJson, bankName)) return 0;
            return importBankJsonFile(p, bankJson);
        }
    } catch (...) { return 0; }
    return 0;
}

int PresetManager::importBankJsonFile(const fs::path& /*p*/, const std::string& bankJson) {
    std::string bankName = bankNameFromJson(bankJson);
    auto presets = splitBankJson(bankJson);
    if (presets.empty()) return 0;
    fs::path bankDir = myBanksFolder() / sanitizeFileName(bankName);
    try { fs::create_directories(bankDir); } catch (...) { return 0; }
    // Write manifest
    writeFileAtomic(bankDir / "bank.json", wrapAsBankJson(bankName, presets));
    int count = 0;
    for (auto& pj : presets) {
        std::string name = extractJsonString(pj, "name");
        if (name.empty()) continue;
        std::string safe = sanitizeFileName(name);
        std::string dest = uniquePath(bankDir, safe, ".json");
        if (writeFileAtomic(dest, pj)) count++;
    }
    return count;
}

int PresetManager::importFiles(const std::vector<fs::path>& dropped) {
    int total = 0;
    for (auto& p : dropped) {
        std::error_code ec;
        if (fs::is_directory(p, ec)) {
            for (auto& e : fs::recursive_directory_iterator(p, ec)) {
                if (e.is_regular_file()) total += importSingleFile(e.path());
            }
        } else if (fs::is_regular_file(p, ec)) {
            total += importSingleFile(p);
        }
    }
    scan();
    return total;
}

// ---------- save / banks ----------
std::string PresetManager::savePreset(const std::string& name, const std::string& category,
                                       const std::string& presetJson, const std::string& bankName) {
    if (name.empty() || presetJson.empty()) return "";
    std::string safe = sanitizeFileName(name);
    fs::path dir = bankName.empty() ? myPresetsFolder() : (myBanksFolder() / sanitizeFileName(bankName));
    try { fs::create_directories(dir); } catch (...) { return ""; }
    // If caller passed a full preset JSON, use it as-is (it already has name/category).
    // If it looks like a fragment, wrap it.
    std::string content = presetJson;
    if (content.find("\"name\"") == std::string::npos) {
        content = makePresetJson(name, category, presetJson);
    }
    std::string dest = uniquePath(dir, safe, ".json");
    if (!writeFileAtomic(dest, content)) return "";
    // If saving into a bank, refresh manifest
    if (!bankName.empty()) {
        auto inBank = presetsInBank(bankName);
        // Re-scan to include the new file, then rebuild manifest
        scan();
        std::vector<std::string> all;
        for (auto& p : presetsInBank(bankName)) all.push_back(p.json);
        writeFileAtomic(dir / "bank.json", wrapAsBankJson(bankName, all));
        (void)inBank;
    } else {
        scan();
    }
    return dest;
}

bool PresetManager::createBank(const std::string& bankName) {
    if (bankName.empty()) return false;
    fs::path dir = myBanksFolder() / sanitizeFileName(bankName);
    std::error_code ec;
    if (fs::exists(dir, ec)) return false; // already exists — don't overwrite
    try {
        fs::create_directories(dir);
        return writeFileAtomic(dir / "bank.json", wrapAsBankJson(bankName, {}));
    } catch (...) { return false; }
}

bool PresetManager::addPresetToBank(const std::string& bankName, const std::string& presetName,
                                    const std::string& presetJson) {
    if (bankName.empty() || presetName.empty() || presetJson.empty()) return false;
    fs::path dir = myBanksFolder() / sanitizeFileName(bankName);
    try { fs::create_directories(dir); } catch (...) { return false; }
    std::string dest = uniquePath(dir, sanitizeFileName(presetName), ".json");
    if (!writeFileAtomic(dest, presetJson)) return false;
    scan();
    std::vector<std::string> all;
    for (auto& p : presetsInBank(bankName)) all.push_back(p.json);
    writeFileAtomic(dir / "bank.json", wrapAsBankJson(bankName, all));
    return true;
}

std::string PresetManager::createBankFromPresets(const std::string& bankName,
                                                 const std::vector<std::string>& presetJsons) {
    if (bankName.empty()) return "";
    fs::path dir = myBanksFolder() / sanitizeFileName(bankName);
    try { fs::create_directories(dir); } catch (...) { return ""; }
    int count = 0;
    for (auto& pj : presetJsons) {
        std::string name = extractJsonString(pj, "name");
        if (name.empty()) continue;
        std::string dest = uniquePath(dir, sanitizeFileName(name), ".json");
        if (writeFileAtomic(dest, pj)) count++;
    }
    if (count == 0) return "";
    writeFileAtomic(dir / "bank.json", wrapAsBankJson(bankName, presetJsons));
    scan();
    return dir.string();
}

bool PresetManager::deleteBank(const std::string& bankName) {
    if (bankName.empty()) return false;
    fs::path dir = myBanksFolder() / sanitizeFileName(bankName);
    std::error_code ec;
    if (!fs::exists(dir, ec)) return false;
    fs::remove_all(dir, ec);
    if (ec) return false;
    scan();
    return true;
}

bool PresetManager::deletePreset(const std::string& filePath) {
    if (filePath.empty()) return false;
    fs::path p(filePath);
    // Safety: only delete inside User folder, never Factory
    std::error_code ec;
    fs::path canon = fs::weakly_canonical(p, ec);
    fs::path userCanon = fs::weakly_canonical(userRoot_, ec);
    if (canon.string().rfind(userCanon.string(), 0) != 0) return false;
    if (canon.string().find("Factory") != std::string::npos) return false;
    bool ok = fs::remove(p, ec);
    if (ok) scan();
    return ok && !ec;
}

bool PresetManager::renamePreset(const std::string& filePath, const std::string& newName) {
    if (filePath.empty() || newName.empty()) return false;
    fs::path p(filePath);
    std::error_code ec;
    fs::path canon = fs::weakly_canonical(p, ec);
    fs::path userCanon = fs::weakly_canonical(userRoot_, ec);
    if (canon.string().rfind(userCanon.string(), 0) != 0) return false;
    if (canon.string().find("Factory") != std::string::npos) return false;
    fs::path dest = p.parent_path() / (sanitizeFileName(newName) + ".json");
    fs::rename(p, dest, ec);
    if (ec) return false;
    scan();
    return true;
}

#!/usr/bin/env python3
"""
Convert MVP JSON presets to VST2 .fxp (single) and .fxb (bank) chunk format.
ONE bank: MVP Presets (2000), organized by type in Presets/<Type>/.
Format (Steinberg CcnK, big-endian):
 .fxp (FPCh chunk preset):
   CcnK | byteSize | FPCh | version=1 | fxID='MVPS' | fxVersion=1 | numPrograms=1 | prgName[28] | chunkSize | chunk(JSON)
 .fxb (FBCh chunk bank):
   CcnK | byteSize | FBCh | version=1 | fxID='MVPS' | fxVersion=1 | numPrograms=N | future[128]=0 | [FPCh preset blocks...]
Chunk = compact JSON of the preset object. Plugin ID MVPS = MVP.
JSON is native; .fxp/.fxb are import-only.
"""
import json, os, struct, re, pathlib

BASE = os.path.dirname(os.path.abspath(__file__))
FXID = b'MVPS'

def pad_name(name, length=28):
    b = name.encode('utf-8')[:length]
    return b + b'\x00' * (length - len(b))

def make_fxp(preset):
    chunk = json.dumps(preset, separators=(',', ':')).encode('utf-8')
    name = pad_name(preset['name'])
    body = b'FPCh' + struct.pack('>i', 1) + FXID + struct.pack('>i', 1) + struct.pack('>i', 1) + name + struct.pack('>i', len(chunk)) + chunk
    return b'CcnK' + struct.pack('>I', len(body)) + body

def make_fxb(presets):
    programs = b''.join(make_fxp(p) for p in presets)
    body = b'FBCh' + struct.pack('>i', 1) + FXID + struct.pack('>i', 1) + struct.pack('>i', len(presets)) + b'\x00'*128 + programs
    return b'CcnK' + struct.pack('>I', len(body)) + body

def safe_filename(preset_id, name):
    safe = re.sub(r'[^A-Za-z0-9_-]+', '_', name).strip('_')
    return f"{preset_id:04d}_{safe}.fxp"

if __name__ == '__main__':
    bank = json.load(open(os.path.join(BASE, 'presets', 'MVP_Presets_2000.json')))
    presets = bank['presets']
    # .fxp organized by type
    out_base = os.path.join(BASE, 'presets', 'fxp_by_type')
    for p in presets:
        d = os.path.join(out_base, p['category'])
        os.makedirs(d, exist_ok=True)
        with open(os.path.join(d, safe_filename(p['id'], p['name'])), 'wb') as f:
            f.write(make_fxp(p))
    # Single .fxb
    os.makedirs(os.path.join(BASE, 'presets', 'fxb'), exist_ok=True)
    with open(os.path.join(BASE, 'presets', 'fxb', 'MVP_Presets_2000.fxb'), 'wb') as f:
        f.write(make_fxb(presets))
    print(f"ONE bank: MVP Presets — {len(presets)} .fxp (by type) + 1 .fxb")
    from collections import Counter
    print(Counter(p['category'] for p in presets))

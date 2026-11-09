from pathlib import Path
base = Path(__file__).resolve().parent
out = ['#pragma once\n// Auto-generated. Do not manually edit; edit web-source files.\n']
for key, filename in [('HTML','index.html'),('CSS','style.css'),('JS','script.js')]:
    content = (base / 'web-source' / filename).read_text(encoding='utf-8')
    delimiter = 'W'+key+'2026'
    if ')'+delimiter+'"' in content:
        raise ValueError('Raw-string delimiter collision')
    out.append(f'const char {key}_DATA[] PROGMEM = R"{delimiter}({content}){delimiter}";\n')
(base / 'RelayESP32' / 'web_assets.h').write_text('\n'.join(out), encoding='utf-8')
print('Updated RelayESP32/web_assets.h')

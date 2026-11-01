"""Validate full Alpine sprite coverage, masks and original canvas alignment."""
from pathlib import Path
import json
from PIL import Image,ImageChops
from export_alpine_sprites import source_image
assets=Path(__file__).resolve().parents[1]/'assets'
original=assets/'game/data';alpine=assets/'game/alpine'
manifest=json.loads((alpine/'manifest.json').read_text())
expected={str(p.relative_to(original)).lower():p for p in original.rglob('*')
          if p.suffix.lower()=='.png' and p.relative_to(original).parts[0].lower()!='backgrounds'}
listed={x['path'] for x in manifest['sprites']}
assert listed==set(expected), f'Missing: {set(expected)-listed}; extra: {listed-set(expected)}'
for rel,src in expected.items():
    im=Image.open(alpine/'data'/rel).convert('RGBA');ref=source_image(src)
    assert im.size==(ref.width*4,ref.height*4),rel
    alpha=ref.getchannel('A').resize(im.size,Image.Resampling.LANCZOS)
    assert ImageChops.difference(alpha,im.getchannel('A')).getbbox() is None,rel
assert Image.open(alpine/'data/backgrounds/train_bg3.png').size[0]>1000
print(f'PASS: all {len(expected)} sprite textures supplied at 4x resolution, original alpha masks and pivots preserved')

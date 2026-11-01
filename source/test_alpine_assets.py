"""Validate full Alpine sprite coverage, masks and original canvas alignment."""
from pathlib import Path
import json,argparse
from PIL import Image,ImageChops
from export_alpine_sprites import source_image
assets=Path(__file__).resolve().parents[1]/'assets'
original=assets/'game/data';ap=argparse.ArgumentParser();ap.add_argument('--theme',choices=['alpine','cartoon','cinematic'],default='alpine');args=ap.parse_args()
alpine=assets/f'game/{args.theme}'
manifest=json.loads((alpine/'manifest.json').read_text())
expected={str(p.relative_to(original)).lower():p for p in original.rglob('*')
          if p.suffix.lower()=='.png' and p.relative_to(original).parts[0].lower()!='backgrounds'}
listed={x['path'] for x in manifest['sprites']}
assert listed==set(expected), f'Missing: {set(expected)-listed}; extra: {listed-set(expected)}'
for rel,src in expected.items():
    im=Image.open(alpine/'data'/rel).convert('RGBA');ref=source_image(src)
    assert im.size==(ref.width*4,ref.height*4),rel
    if args.theme in ('cartoon','cinematic'):
        box=im.getchannel('A').getbbox();assert box is not None,rel
        target=ref.getchannel('A').getbbox()
        if 'game_over' in rel:
            target=ref.convert('RGB').convert('L').point(lambda v:255 if v>15 else 0).getbbox()
        target=tuple(v*4 for v in target)
        assert box[0]>=target[0]-2 and box[1]>=target[1]-2 and box[2]<=target[2]+2 and box[3]<=target[3]+2,rel
        continue
    alpha=ref.getchannel('A').resize(im.size,Image.Resampling.LANCZOS)
    assert ImageChops.difference(alpha,im.getchannel('A')).getbbox() is None,rel
if args.theme=='alpine':assert Image.open(alpine/'data/backgrounds/train_bg3.png').size[0]>1000
print(f'PASS: all {len(expected)} {args.theme} textures at 4x resolution, sprite masks and UI placement validated')

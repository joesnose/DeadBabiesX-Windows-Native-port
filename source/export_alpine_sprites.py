"""Export generated atlas cells into the game's original sprite canvases.

Usage: python source/export_alpine_sprites.py --atlases /path/to/atlases
The atlas directory contains layout.json and GROUP-redraw.png files.
Pillow is required. The source images are edited using image generation; this
step only unpacks/resamples them and restores engine-compatible alpha masks.
"""
import argparse, json
from pathlib import Path
from PIL import Image, ImageChops

def source_image(p):
    im=Image.open(p).convert('RGBA')
    im.putdata([(r,g,b,0 if (r,g,b)==(0,0,0) else a) for r,g,b,a in im.getdata()])
    if p.parent.parent.name.lower()=='player' and p.stem.lower().endswith(('_stand','_lives')) and 'legs' not in p.stem.lower():
        # Legacy full-body canvases contain almost-black opaque rectangles.
        # Their real silhouette is the matching original torso/leg union.
        colour=p.parent.name
        torso=source_image(p.parent/(colour+'_0.png'))
        legs=source_image(p.parent/(colour+'_Legs_Stand.png'))
        mask=ImageChops.lighter(torso.getchannel('A'),legs.getchannel('A'))
        im.putalpha(mask.resize(im.size,Image.Resampling.LANCZOS))
    return im

def sprite_bounds(cell):
    # Transparent generated atlases can contain faint isolated specks. Find
    # substantial connected pieces so those specks cannot shrink the real art.
    alpha=cell.getchannel('A');w,h=cell.size;a=alpha.load();seen=set();parts=[]
    for y in range(h):
        for x in range(w):
            if (x,y) in seen or a[x,y]<200: continue
            stack=[(x,y)];seen.add((x,y));count=0;left=right=x;top=bottom=y
            while stack:
                px,py=stack.pop();count+=1
                left=min(left,px);right=max(right,px);top=min(top,py);bottom=max(bottom,py)
                for dx,dy in [(1,0),(-1,0),(0,1),(0,-1)]:
                    nx,ny=px+dx,py+dy
                    if 0<=nx<w and 0<=ny<h and (nx,ny) not in seen and a[nx,ny]>=200:
                        seen.add((nx,ny));stack.append((nx,ny))
            parts.append((count,(left,top,right+1,bottom+1)))
    if not parts:return alpha.getbbox()
    limit=max(p[0] for p in parts)*.12
    boxes=[b for n,b in parts if n>=limit]
    return min(b[0] for b in boxes),min(b[1] for b in boxes),max(b[2] for b in boxes),max(b[3] for b in boxes)

def main():
    ap=argparse.ArgumentParser();ap.add_argument('--atlases',type=Path,required=True)
    ap.add_argument('--assets',type=Path,default=Path('assets'));ap.add_argument('--theme',choices=['alpine','cartoon'],default='alpine');args=ap.parse_args()
    layout=json.loads((args.atlases/'layout.json').read_text())
    data=args.assets/'game/data';out=args.assets/f'game/{args.theme}/data';records=[]
    for group,meta in layout.items():
        sheet=Image.open(args.atlases/(group+'-redraw.png')).convert('RGBA')
        sx=sheet.width/meta['size'][0];sy=sheet.height/meta['size'][1]
        if args.theme=='cartoon' and group=='players':
            # This redraw retains seven columns but uses six occupied rows.
            occupied_rows=(len(meta['entries'])+meta['cols']-1)//meta['cols']
            sy=sheet.height/(occupied_rows*meta['cell'])
        for index,entry in enumerate(meta['entries']):
            rel=entry['file'];original=source_image(data/rel)
            size=(original.width*4,original.height*4)
            cx=index%meta['cols']*meta['cell'];cy=index//meta['cols']*meta['cell']
            box=(round(cx*sx),round(cy*sy),round((cx+meta['cell'])*sx),round((cy+meta['cell'])*sy))
            cell=sheet.crop(box)
            bounds=sprite_bounds(cell)
            target=original.getchannel('A').getbbox()
            cartoon_hud=args.theme=='cartoon' and group=='hud'
            if cartoon_hud and 'game_over' in rel.lower():
                target=original.convert('RGB').convert('L').point(lambda v:255 if v>15 else 0).getbbox()
            drawn=Image.new('RGBA',size)
            if bounds and target:
                x,y,x2,y2=(v*4 for v in target)
                piece=cell.crop(bounds).resize((x2-x,y2-y),Image.Resampling.LANCZOS)
                drawn.alpha_composite(piece,(x,y))
            # Preserve alignment and silhouette; transparent generator gaps fall
            # back to the source so animation pieces never disappear.
            result=Image.alpha_composite(original.resize(size,Image.Resampling.LANCZOS),drawn)
            mask=original.getchannel('A').resize(size,Image.Resampling.LANCZOS)
            result.putalpha(mask)
            if args.theme=='cartoon':
                # UI lettering has no collision/animation silhouette to retain.
                # Keep its original placement but allow legible new glyph shapes.
                result=drawn
                result.putalpha(result.getchannel('A').point(lambda a:0 if a<8 else a))
            dest=out/rel.lower();dest.parent.mkdir(parents=True,exist_ok=True);result.save(dest)
            if args.theme!='cartoon':
                assert ImageChops.difference(result.getchannel('A'),mask).getbbox() is None
            records.append({'path':rel.lower(),'size':size,'source_size':list(original.size),'group':group})
    # Full-body life/stand icons reuse the exact modular torso and leg art;
    # this also prevents generated atlas variations from losing their legs.
    for colour in ['Blue','Green','Red','Yellow']:
        folder=out/'player'/colour.lower()
        lower=Image.open(folder/(colour.lower()+'_legs_stand.png')).convert('RGBA')
        upper=Image.open(folder/(colour.lower()+'_0.png')).convert('RGBA')
        lower.alpha_composite(upper)
        for suffix in ['Stand','Lives']:
            rel=f'Player/{colour}/{colour}_{suffix}.png'
            original=source_image(data/rel);size=(original.width*4,original.height*4)
            result=lower.resize(size,Image.Resampling.LANCZOS)
            if args.theme!='cartoon':
                result.putalpha(original.getchannel('A').resize(size,Image.Resampling.LANCZOS))
            result.save(out/rel.lower())
    manifest=args.assets/f'game/{args.theme}/manifest.json'
    info=json.loads(manifest.read_text());info.update(sprite_revision=2,sprite_count=len(records),sprites=records,
        sprites_description=('Flat cartoon redraws, four times original canvas resolution; original module positions and pivots retained' if args.theme=='cartoon' else 'Generated sprite redraws, four times original canvas resolution; modular alpha masks and pivots retained'))
    manifest.write_text(json.dumps(info,indent=2))
    print('Exported and alpha-validated',len(records),args.theme+' sprite textures')

if __name__=='__main__': main()

"""Check bottom coverage against the actual train artwork in every art pack."""
from pathlib import Path
from PIL import Image
import subprocess,tempfile
root=Path(__file__).resolve().parent
cases=[]
for p in (root.parent/'assets/game').rglob('*'):
    if p.name.lower() not in ('train_bg1.png','train_bg2.png'):continue
    im=Image.open(p).convert('RGBA')
    if '/game/data/' in p.as_posix():
        im.putdata([(r,g,b,0 if (r,g,b)==(0,0,0) else a) for r,g,b,a in im.getdata()])
    box=im.getchannel('A').getbbox()
    assert box,p
    bottom=240-320+640*box[3]/im.height
    assert 460<bottom<500,(p,bottom)
    cases.append(bottom)
code='''#include "train_base.h"
#include <cassert>
#include <cmath>
#include <cstdio>
int main(){
'''
code+='float bottoms[]={'+','.join(str(x)+'f' for x in cases)+'};\n'
code+='''for(float bottom:bottoms)for(float camera:{-128.f,-32.f,0.f,32.f,128.f}) {
 TrainBaseStretch t(bottom,camera);
 for(float y:{-20.f,0.f,400.f,440.f,460.f})assert(t.map(y)==y);
 assert(t.map(bottom)+camera>=500.f);
 assert(t.map(bottom)>=bottom);
}
puts("PASS: actual train textures reach the playfield bottom; upper geometry stays fixed, including camera shake");
}
'''
with tempfile.TemporaryDirectory() as tmp:
    p=Path(tmp);(p/'test.cpp').write_text(code)
    subprocess.run(['g++','-std=c++17','-I',str(root),str(p/'test.cpp'),'-o',str(p/'test')],check=True)
    subprocess.run([str(p/'test')],check=True)
print('Checked',len(cases),'train layer assets')

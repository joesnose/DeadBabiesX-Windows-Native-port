"""Exercise the actual menu input/action implementation with a renderer stub."""
from pathlib import Path
import subprocess, tempfile
root=Path(__file__).resolve().parent
s=(root/'frontend.cpp').read_text()
update=s[s.index('MenuAction Frontend::Update()'):s.index('MenuAction Frontend::ToggleMusic')]
code=r'''
#include <cassert>
#include <cstdio>
#include "difficulty.h"
enum class MenuAction {None,Resume,Title,Exit,Start,ArtChanged};
struct Pad {void*hDevice=nullptr;int wButtons=0,wPressedButtons=0;int bAnalogButtons[8]{},bPressedAnalogButtons[8]{};float fX1=0,fY1=0,fX2=0,fY2=0;};
struct Engine {Pad GamePad[4];bool back=false;bool TakeMenuBack(){bool v=back;back=false;return v;}} engine;
auto* XEngine=&engine;
struct Frontend {bool paused=false,inputArmed=true;Difficulty difficulty=Difficulty::Normal;int selection=0,heldDirection=0,heldHorizontal=0,players=1;
MenuAction Update();MenuAction ToggleArt(int){return MenuAction::ArtChanged;}MenuAction ToggleMusic(int){return MenuAction::None;}};
'''+update+r'''
int main(){
 engine.GamePad[0].hDevice=(void*)1;
 engine.GamePad[0].bPressedAnalogButtons[0]=1;
 Frontend f;f.paused=true;f.selection=3;assert(f.Update()==MenuAction::Title);
 f.paused=false;f.selection=5;assert(f.Update()==MenuAction::Exit);
 f.paused=true;f.selection=0;assert(f.Update()==MenuAction::Resume);
 engine.GamePad[0].bPressedAnalogButtons[0]=0;engine.GamePad[0].wPressedButtons=2;
 f.selection=3;f.heldDirection=0;assert(f.Update()==MenuAction::None);assert(f.selection==0);
 engine.GamePad[0].wPressedButtons=0;engine.back=true;assert(f.Update()==MenuAction::Resume);
 f.inputArmed=false;engine.GamePad[0].bPressedAnalogButtons[0]=1;
 engine.GamePad[0].bAnalogButtons[0]=255;f.selection=3;assert(f.Update()==MenuAction::None);
 engine.GamePad[0]=Pad{};engine.GamePad[0].hDevice=(void*)1;
 Frontend d;d.selection=4;assert(d.difficulty==Difficulty::Normal);
 engine.GamePad[0].bPressedAnalogButtons[0]=1;d.Update();assert(d.difficulty==Difficulty::Hard);
 d.Update();assert(d.difficulty==Difficulty::Easy);d.Update();assert(d.difficulty==Difficulty::Normal);
 engine.GamePad[0].bPressedAnalogButtons[0]=0;engine.GamePad[0].wPressedButtons=4;
 d.Update();assert(d.difficulty==Difficulty::Easy);
 puts("PASS: difficulty confirm/left cycling and Normal default");
 puts("PASS: pause Exit returns to title, title Exit quits, resume/back/wrap and held-confirm guard");
}
'''
with tempfile.TemporaryDirectory() as tmp:
 p=Path(tmp);(p/'test.cpp').write_text(code)
 subprocess.run(['g++','-std=c++17','-Wall','-Wextra','-I',str(root),str(p/'test.cpp'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)

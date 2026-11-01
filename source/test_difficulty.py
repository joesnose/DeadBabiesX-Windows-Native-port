"""Compile actual difficulty configuration and enemy spawn code with engine stubs."""
from pathlib import Path
import subprocess,tempfile
root=Path(__file__).resolve().parent
game=(root/'game.cpp').read_text()
config=game[game.index('static bool megaWeapons'):game.index('int CWeapon::Upgrade')]
constructor=game[game.index('CBaby::CBaby()'):game.index('void CBaby::Damage')]
code=r'''
#include "difficulty.h"
#include <cassert>
#include <cstdio>
#include <cstddef>
float Rand(){return .25f;}
enum {BS_FLOATINGLEFT,BS_FLOATINGRIGHT};
struct V{float x=0,y=0;};
struct CBaby {void*m_LastHit=nullptr,*m_Killer=nullptr;V m_Speed,m_Pos;
 float m_Rot=0,m_DeltaRot=0,m_Stand=0;int m_NumBalloons=0,m_ThreatLevel=0,m_Health=0,m_Hit=0,m_BabyState=0;
 bool m_HasBomb=false,m_OnFire=false;CBaby();};
'''+config+constructor+r'''
int main(){
 assert(WeaponPowerLimit()==3);
 for(auto d:{Difficulty::Easy,Difficulty::Normal,Difficulty::Hard}){
  SetGameDifficulty(d);
  assert(WeaponPowerLimit()==(d==Difficulty::Easy?5:3));
  for(int i=0;i<100;++i){CBaby b;assert(b.m_HasBomb==(d==Difficulty::Hard));}
  assert(DifficultyCanDropBomb(d,0)==(d==Difficulty::Hard));
  assert(DifficultyCanDropBomb(d,1));
 }
 SetGameDifficulty(Difficulty::Easy);SetGameDifficulty(Difficulty::Hard);assert(WeaponPowerLimit()==3);
 SetGameDifficulty(Difficulty::Normal);CBaby normal;assert(!normal.m_HasBomb);
 assert(CycleDifficulty(Difficulty::Normal,-1)==Difficulty::Easy);
 assert(CycleDifficulty(Difficulty::Hard,1)==Difficulty::Easy);
 puts("PASS: real enemy constructor arms all Hard spawns, standard spawn rules preserved, weapon caps and mode reset");
}
'''
code=code.replace('#include <cassert>','#include <cassert>\n#include <initializer_list>')
with tempfile.TemporaryDirectory() as tmp:
    p=Path(tmp);(p/'test.cpp').write_text(code)
    subprocess.run(['g++','-std=c++17','-Wall','-Wextra','-I',str(root),str(p/'test.cpp'),'-o',str(p/'test')],check=True)
    subprocess.run([str(p/'test')],check=True)

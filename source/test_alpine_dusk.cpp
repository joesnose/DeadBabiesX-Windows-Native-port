#include <cmath>
#include <cassert>
#include <cstdio>
#include "alpine_dusk.h"
int main() {
  std::vector<unsigned char> p(16*16*4);
  for(size_t i=0;i<p.size();i+=4){p[i]=180;p[i+1]=40;p[i+2]=70;p[i+3]=(i/4)%3?255:0;}
  auto original=p;
  AlpineDuskPixels(p,16,16,"game/hd/data/player/red/red_0.png");
  assert(p!=original);
  for(size_t i=3;i<p.size();i+=4) assert(p[i]==original[i]);
  auto first=p;p=original;
  AlpineDuskPixels(p,16,16,"game/hd/data/player/red/red_0.png");
  assert(p==first);
  p=original;AlpineDuskPixels(p,16,16,"game/data/GUI/gui_box.png");assert(p==original);
  p=original;AlpineDuskPixels(p,16,16,"game/data/Aimers/aimer_red.png");assert(p==original);
  p=original;AlpineDuskPixels(p,16,16,"game/data/Backgrounds/train_bg2.png");assert(p!=original);
  for(size_t i=3;i<p.size();i+=4) assert(p[i]==original[i]);
  puts("PASS: alpine palette, repeatability, exact alpha preservation, HUD and aiming colours");
}

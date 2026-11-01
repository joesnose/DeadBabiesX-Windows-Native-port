#pragma once
#include <algorithm>
#include <cctype>
#include <cmath>
#include <string>
#include <vector>

// Load-time stylisation; alpha and sprite canvases stay untouched.
inline void AlpineDuskPixels(std::vector<unsigned char>& pixels, unsigned w,
                             unsigned h, std::string name) {
  for (char& c : name) c = char(std::tolower(static_cast<unsigned char>(c)));
  const bool terrain = name.find("backgrounds/") != std::string::npos;
  if (!terrain && name.find("player/") == std::string::npos &&
      name.find("babies/") == std::string::npos &&
      name.find("crates/") == std::string::npos &&
      name.find("gibs/") == std::string::npos) return;
  const auto source = pixels;
  const unsigned cell = std::max(2u, w / (terrain ? 32u : 18u));
  for (unsigned y=0; y<h; ++y) for (unsigned x=0; x<w; ++x) {
    const size_t i = (size_t(y)*w+x)*4;
    if (!pixels[i+3]) continue;
    const unsigned bx=x/cell, by=y/cell;
    const bool triangle=(x%cell)+(y%cell)>=cell;
    const float facet = (int((bx*13+by*7+triangle*3)%7)-3)*.024f;
    if (terrain) {
      // Cool faceted stone and moss in place of the industrial ground palette.
      const float lum=(source[i]*.2126f+source[i+1]*.7152f+source[i+2]*.0722f)/255.f;
      const float shade=std::clamp(.40f+std::round(lum*3.f)*.12f+facet,0.f,1.f);
      const bool front=name.find("bg1")!=std::string::npos;
      const float base[3]={front?.30f:.57f,front?.43f:.61f,front?.40f:.70f};
      for(int k=0;k<3;++k) pixels[i+k]=static_cast<unsigned char>(std::clamp(base[k]*shade*1.65f,0.f,1.f)*255);
    } else {
      // Fewer tonal bands, blue shadows, and warm highlights; retain team hues.
      for(int k=0;k<3;++k) {
        float v=source[i+k]/255.f;
        v=std::round(v*5.f)/5.f;
        v=v*.86f+(.05f+(k==2?.035f:0.f))*(1.f-v)+(k==0?.06f:.025f)*v+facet;
        pixels[i+k]=static_cast<unsigned char>(std::clamp(v,0.f,1.f)*255);
      }
    }
  }
}

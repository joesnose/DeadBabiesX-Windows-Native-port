#pragma once
#include "game.h"
enum class MenuAction { None, Start, Resume, Title, ArtChanged, Exit };
class Frontend {
public:
  bool playing = false, paused = false;
  Frontend();
  void OpenPause();
  void OpenTitle();
  void Play();
  bool WantsPause();
  MenuAction Update();
  int PlayerCount() const { return players; }
  Difficulty SelectedDifficulty() const { return difficulty; }
  void Draw();

private:
  int selection = 0, heldDirection = 0, heldHorizontal = 0, players = 1;
  bool inputArmed = false;
  int artMode = 0;
  Difficulty difficulty = Difficulty::Normal;
  MenuAction ToggleArt(int delta);
  MenuAction ToggleMusic(int delta);
  IDirect3DTexture8 *backdrop = nullptr;
};

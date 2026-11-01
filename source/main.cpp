#include "frontend.h"
#include "game.h"
#include <exception>
#include <string>
extern void ResetNativeRound();
extern int NativeSelfTest();
extern int NativeAudioSelfTest();
extern void NativeStopAudio();
static LONG WINAPI crash(EXCEPTION_POINTERS *ep) {
  fprintf(stderr, "CRASH: exception %08lX at %p\n",
          ep->ExceptionRecord->ExceptionCode,
          ep->ExceptionRecord->ExceptionAddress);
  fflush(stderr);
  return EXCEPTION_CONTINUE_SEARCH;
}
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR args, int) {
  FILE *log = freopen("native.log", "w", stderr);
  if (log)
    setvbuf(log, nullptr, _IONBF, 0);
  SetUnhandledExceptionFilter(crash);
  if (args && std::string(args) == "--self-test")
    return NativeSelfTest();
  if (args && std::string(args) == "--audio-self-test")
    return NativeAudioSelfTest();
  try {
    XEngine->XInit();
    Frontend frontend;
    XEngine->PlayMusic("d:\\data\\music\\music3.wma");
    bool roundExists = false;
    if (getenv("DBX_STRESS")) {
      XEngine->StopAllSounds();
      SetGameDifficulty(frontend.SelectedDifficulty());
      Game.Init();
      roundExists = true;
      frontend.Play();
    }
    bool resetTestDone = false;
    while (XEngine->Running()) {
      if (!frontend.playing) {
        MenuAction action = frontend.Update();
        if (action == MenuAction::Exit) {
          fprintf(stderr, "MENU exit\n");
          XEngine->RequestExit();
          continue;
        }
        if (action == MenuAction::Start) {
          fprintf(stderr, "MENU start\n");
          SetStartingPlayers(frontend.PlayerCount());
          SetGameDifficulty(frontend.SelectedDifficulty());
          if (roundExists)
            ResetNativeRound();
          else {
            XEngine->StopAllSounds();
            Game.Init();
            roundExists = true;
          }
          frontend.Play();
          continue;
        }
        if (action == MenuAction::ArtChanged && roundExists) {
          Game.LoadArt();
          auto *player = Game.m_PlayerList.empty() ? nullptr : Game.m_PlayerList.front();
          fprintf(stderr, "ART live reload complete; score=%d players=%d x=%.2f y=%.2f power=%d\n",
                  player ? player->m_Score : 0, Game.m_NumPlayers,
                  player ? player->m_PlayerPos.x : 0, player ? player->m_PlayerPos.y : 0,
                  player ? player->m_Weapon->m_PowerLevel : 0);
        }
        if (action == MenuAction::Resume) {
          fprintf(stderr, "MENU resume\n");
          frontend.Play();
          continue;
        }
        if (action == MenuAction::Title) {
          XEngine->SetAudioPaused(false);
          XEngine->StopAllSounds(false);
          XEngine->PlayMusic("d:\\data\\music\\music3.wma");
          frontend.OpenTitle();
        }
        frontend.Draw();
        XEngine->XUpdate();
        continue;
      }
      if (frontend.WantsPause()) {
        frontend.OpenPause();
        continue;
      }
      if (getenv("DBX_STRESS"))
        for (auto *player : Game.m_PlayerList) {
          player->m_PlayerCondition = PC_INVINCIBLE;
          player->m_ConditionTime = 300;
        }
      if (const char *reset = getenv("DBX_TEST_RESTART_AT")) {
        if (!resetTestDone && XEngine->m_Ticks >= atof(reset) * 60) {
          ResetNativeRound();
          resetTestDone = true;
          fprintf(stderr, "TEST round restart completed\n");
        }
      }
      if (getenv("DBX_TEST_PICKUPS"))
        XEngine->GamePad[0].bAnalogButtons[0] = 255;
      if (!Game.Update()) {
        XEngine->StopAllSounds(false);
        XEngine->PlayMusic("d:\\data\\music\\music3.wma");
        frontend.OpenTitle();
      }
    }
    XEngine->Shutdown();
    return 0;
  } catch (const std::exception &e) {
    NativeStopAudio();
    fprintf(stderr, "FATAL: %s\n", e.what());
    MessageBoxA(nullptr, e.what(), "Dead Babies X Native", MB_ICONERROR);
    return 1;
  }
}

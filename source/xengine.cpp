#include "xengine.h"
#include "cel_shading.h"
#include "cartoon_shading.h"
#include "alpine_dusk.h"
#undef XInputSetState
#include <GL/gl.h>
#include <mmsystem.h>
#include <wincodec.h>
#include <xinput.h>
#undef PlaySound
#include <array>
#include <atomic>
#include <chrono>
#include <cstring>
#include <fstream>
#include <map>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
extern "C" int stb_vorbis_decode_filename(const char *, int *, int *, short **);
static HWND window;
static HDC dc;
static HGLRC rc;
static bool running = true;
static bool menuBack = false;
static LARGE_INTEGER frequency, lastFrame;
static unsigned long long frames = 0;
static double seconds = 0;
static IWICImagingFactory *wic;
static GLuint fontTexture;
static int viewportW = 960, viewportH = 720;
static void applyViewport() {
  int vw = viewportW, vh = viewportH;
  if (vw * 3 > vh * 4)
    vw = vh * 4 / 3;
  else
    vh = vw * 3 / 4;
  glViewport((viewportW - vw) / 2, (viewportH - vh) / 2, vw, vh);
  static int lastWidth = -1, lastHeight = -1;
  if (lastWidth != viewportW || lastHeight != viewportH) {
    fprintf(stderr, "Display: %dx%d; 4:3 game viewport: %dx%d at %d,%d\n",
            viewportW, viewportH, vw, vh, (viewportW - vw) / 2,
            (viewportH - vh) / 2);
    lastWidth = viewportW;
    lastHeight = viewportH;
  }
}
static std::map<std::string, std::unique_ptr<IDirect3DTexture8>> textures;
CXEngine *XEngine = new CXEngine;
static std::wstring wide(const std::string &s) {
  int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
  std::wstring w(n, 0);
  MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, w.data(), n);
  return w;
}
static std::string path(const char *name) {
  std::string p = name;
  if (p.size() > 2 && p[1] == ':')
    p = p.substr(3);
  for (char &c : p)
    if (c == '\\')
      c = '/';
  if (GetFileAttributesA(p.c_str()) != INVALID_FILE_ATTRIBUTES)
    return p;
  return "game/" + p;
}
static bool key(int k) {
  return (GetAsyncKeyState(k) & 0x8000) != 0 && GetForegroundWindow() == window;
}
static void fullscreen() {
  static WINDOWPLACEMENT saved{sizeof(saved)};
  LONG style = GetWindowLongA(window, GWL_STYLE);
  if (style & WS_OVERLAPPEDWINDOW) {
    GetWindowPlacement(window, &saved);
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoA(MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST), &mi);
    SetWindowLongA(window, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
    SetWindowPos(window, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
                 mi.rcMonitor.right - mi.rcMonitor.left,
                 mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_FRAMECHANGED);
  } else {
    SetWindowLongA(window, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
    SetWindowPlacement(window, &saved);
    SetWindowPos(window, nullptr, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
  }
}
static LRESULT CALLBACK proc(HWND h, UINT msg, WPARAM w, LPARAM l) {
  if (msg == WM_CLOSE) {
    running = false;
    return 0;
  }
  if (msg == WM_SIZE) {
    viewportW = LOWORD(l);
    viewportH = HIWORD(l);
    return 0;
  }
  if ((msg == WM_KEYDOWN && w == VK_F11) ||
      (msg == WM_SYSKEYDOWN && w == VK_RETURN)) {
    if (!(l & (1 << 30)))
      fullscreen();
    return 0;
  }
  if (msg == WM_KEYDOWN && w == VK_ESCAPE) {
    if (!(l & (1 << 30)))
      menuBack = true;
    return 0;
  }
  return DefWindowProcA(h, msg, w, l);
}
using Voice = NativeAudioVoice;
static std::mutex audioMutex;
static std::vector<Voice> voices;
static uint64_t nextVoice = 1;
static uint64_t musicVoiceId = 0;
static int musicVolume = 1000;
static int musicChoice = 0;
int NativeMusicChoice() { return musicChoice; }
const char *NativeMusicLabel(int choice) { return choice == 3 ? "ROCK" : choice == 2 ? "ACID HOUSE" : choice == 1 ? "STATIC REACTOR" : "ORIGINAL"; }
static bool musicPaused = false;
static std::vector<std::unique_ptr<IDirectMusicSegmentState>> tokens;
void CXEngine::StopAllSounds(bool releaseTokens) {
  std::lock_guard<std::mutex> lock(audioMutex);
  voices.clear();
  musicVoiceId = 0;
  musicPaused = false;
  if (releaseTokens)
    tokens.clear();
}
static bool audioPaused = false;
static HWAVEOUT audioDevice;
static HANDLE audioDoneEvent;
static std::thread audioThread;
static std::atomic<bool> audioStopping{false};
static WAVEHDR headers[4]{};
static short buffers[4][1024 * 2];
static void fill(WAVEHDR *header) {
  std::lock_guard<std::mutex> lock(audioMutex);
  NativeMixAudio(voices, reinterpret_cast<short *>(header->lpData),
                 header->dwBufferLength / (sizeof(short) * 2), audioPaused);
}
static void shutdownAudio() {
  audioStopping.store(true);
  if (audioDoneEvent) SetEvent(audioDoneEvent);
  if (audioThread.joinable()) audioThread.join();
  if (audioDevice) {
    waveOutReset(audioDevice);
    for (auto &header : headers)
      if (header.dwFlags & WHDR_PREPARED)
        waveOutUnprepareHeader(audioDevice, &header, sizeof(header));
    waveOutClose(audioDevice);
    audioDevice = nullptr;
  }
  if (audioDoneEvent) {
    CloseHandle(audioDoneEvent);
    audioDoneEvent = nullptr;
  }
}
void NativeStopAudio() { shutdownAudio(); }
static void initAudio() {
  audioDoneEvent = CreateEventA(nullptr, FALSE, FALSE, nullptr);
  if (!audioDoneEvent) {
    fprintf(stderr, "Audio event unavailable\n");
    return;
  }
  WAVEFORMATEX fmt{WAVE_FORMAT_PCM, 2, 48000, 192000, 4, 16, 0};
  if (waveOutOpen(&audioDevice, WAVE_MAPPER, &fmt, (DWORD_PTR)audioDoneEvent, 0,
                  CALLBACK_EVENT) != MMSYSERR_NOERROR) {
    audioDevice = nullptr;
    CloseHandle(audioDoneEvent);
    audioDoneEvent = nullptr;
    fprintf(stderr, "Audio device unavailable\n");
    return;
  }
  for (int i = 0; i < 4; ++i) {
    headers[i] = {};
    headers[i].lpData = reinterpret_cast<char *>(buffers[i]);
    headers[i].dwBufferLength = sizeof(buffers[i]);
    MMRESULT status = waveOutPrepareHeader(audioDevice, &headers[i], sizeof(WAVEHDR));
    if (status == MMSYSERR_NOERROR) {
      fill(&headers[i]);
      status = waveOutWrite(audioDevice, &headers[i], sizeof(WAVEHDR));
    }
    if (status != MMSYSERR_NOERROR) {
      fprintf(stderr, "Audio queue initialization failed: %u\n", unsigned(status));
      shutdownAudio();
      return;
    }
  }
  audioStopping.store(false);
  // Windows signals completed buffers; refill and submit from this worker,
  // never from a waveOut callback where reentering waveOut can deadlock.
  audioThread = std::thread([] {
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
    while (!audioStopping.load()) {
      DWORD result = WaitForSingleObject(audioDoneEvent, 1000);
      if (audioStopping.load()) break;
      if (result == WAIT_FAILED) {
        fprintf(stderr, "Audio event wait failed: %lu\n", GetLastError());
        break;
      }
      for (auto &header : headers) {
        if (!(header.dwFlags & WHDR_DONE)) continue;
        fill(&header);
        MMRESULT status = waveOutWrite(audioDevice, &header, sizeof(header));
        if (status != MMSYSERR_NOERROR) {
          fprintf(stderr, "Audio buffer submission failed: %u\n", unsigned(status));
          return;
        }
      }
    }
  });
}
static uint32_t u32(const unsigned char *p) {
  uint32_t v;
  memcpy(&v, p, 4);
  return v;
}
void CSound::Create(const char *name) {
  static std::map<std::string, std::shared_ptr<SoundData>> cache;
  std::string filename = path(name);
  auto found = cache.find(filename);
  if (found != cache.end()) {
    sound = found->second;
    return;
  }
  if (filename.size() > 4 && filename.substr(filename.size() - 4) == ".ogg") {
    int channels = 0, rate = 0;
    short *pcm = nullptr;
    int count =
        stb_vorbis_decode_filename(filename.c_str(), &channels, &rate, &pcm);
    sound = std::make_shared<SoundData>();
    if (count > 0 && channels > 0 && rate > 0) {
      size_t n = (size_t)count * 48000 / rate;
      sound->samples.resize(n * 2);
      for (size_t i = 0; i < n; i++)
        for (int ch = 0; ch < 2; ch++)
          sound->samples[i * 2 + ch] =
              pcm[std::min((size_t)count - 1, i * rate / 48000) * channels +
                  std::min(ch, channels - 1)] /
              32768.f;
      cache[filename] = sound;
    }
    free(pcm);
    return;
  }
  std::ifstream f(filename, std::ios::binary);
  std::vector<unsigned char> b((std::istreambuf_iterator<char>(f)), {});
  sound = std::make_shared<SoundData>();
  if (b.size() < 12 || memcmp(b.data(), "RIFF", 4)) {
    fprintf(stderr, "Sound missing/invalid: %s\n", filename.c_str());
    return;
  }
  unsigned channels = 1, rate = 44100, bits = 16, format = 1;
  size_t at = 12, data = 0, len = 0;
  while (at + 8 <= b.size()) {
    size_t n = u32(b.data() + at + 4);
    if (n > b.size() - at - 8)
      break;
    if (!memcmp(b.data() + at, "fmt ", 4) && n >= 16) {
      format = b[at + 8] | b[at + 9] << 8;
      channels = b[at + 10] | b[at + 11] << 8;
      rate = u32(b.data() + at + 12);
      bits = b[at + 22] | b[at + 23] << 8;
    }
    if (!memcmp(b.data() + at, "data", 4)) {
      data = at + 8;
      len = n;
    }
    at += 8 + n + (n & 1);
  }
  if (format != 1 || channels < 1 || channels > 2 ||
      (bits != 8 && bits != 16) || !rate || !data) {
    fprintf(stderr, "Unsupported WAV: %s\n", filename.c_str());
    return;
  }
  size_t count = len / (channels * (bits / 8)),
         framesOut = count * 48000 / rate;
  sound->samples.resize(framesOut * 2);
  for (size_t i = 0; i < framesOut; i++)
    for (unsigned ch = 0; ch < 2; ch++) {
      size_t sample = std::min(count - 1, i * rate / 48000) * channels +
                      std::min(ch, channels - 1);
      float v = bits == 16 ? (float)(int16_t)(b[data + sample * 2] |
                                              b[data + sample * 2 + 1] << 8) /
                                 32768.0f
                           : ((int)b[data + sample] - 128) / 128.0f;
      sound->samples[i * 2 + ch] = v;
    }
  cache[filename] = sound;
}
void CSound::PlaySound() {
  if (!sound || sound->samples.empty() || XEngine->m_SoundsPerFrame <= 0)
    return;
  XEngine->m_SoundsPerFrame--;
  std::lock_guard<std::mutex> lock(audioMutex);
  if (voices.size() < 96)
    voices.push_back({sound, 0, loop, nextVoice++});
}
void CSound::PlaySound(IDirectMusicSegmentState **channel) {
  if (!channel)
    return;
  std::lock_guard<std::mutex> lock(audioMutex);
  auto token = std::make_unique<IDirectMusicSegmentState>();
  token->id = nextVoice++;
  *channel = token.get();
  if (sound && !sound->samples.empty() && voices.size() < 96)
    voices.push_back({sound, 0, loop, token->id});
  tokens.push_back(std::move(token));
}
void CXEngine::StopChannel(IDirectMusicSegmentState *channel) {
  if (!channel)
    return;
  std::lock_guard<std::mutex> lock(audioMutex);
  voices.erase(std::remove_if(voices.begin(), voices.end(),
                              [&](auto &v) { return v.id == channel->id; }),
               voices.end());
}
static void makeFont() {
  HDC mem = CreateCompatibleDC(dc);
  BITMAPINFO bi{};
  bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bi.bmiHeader.biWidth = 512;
  bi.bmiHeader.biHeight = -256;
  bi.bmiHeader.biPlanes = 1;
  bi.bmiHeader.biBitCount = 32;
  void *pixels;
  HBITMAP bm = CreateDIBSection(mem, &bi, DIB_RGB_COLORS, &pixels, nullptr, 0);
  HGDIOBJ old = SelectObject(mem, bm);
  HFONT font =
      CreateFontA(-22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,
                  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                  FIXED_PITCH, "Consolas");
  HGDIOBJ oldfont = SelectObject(mem, font);
  memset(pixels, 0, 512 * 256 * 4);
  SetTextColor(mem, RGB(255, 255, 255));
  SetBkColor(mem, 0);
  for (int i = 32; i < 127; i++) {
    char ch = (char)i;
    TextOutA(mem, (i % 16) * 32, (i / 16 - 2) * 32, &ch, 1);
  }
  std::vector<unsigned char> rgba(512 * 256 * 4);
  for (size_t i = 0; i < rgba.size() / 4; i++) {
    unsigned char *px = (unsigned char *)pixels + i * 4;
    rgba[i * 4] = rgba[i * 4 + 1] = rgba[i * 4 + 2] = 255;
    rgba[i * 4 + 3] = std::max({px[0], px[1], px[2]});
  }
  glGenTextures(1, &fontTexture);
  glBindTexture(GL_TEXTURE_2D, fontTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 256, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, rgba.data());
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  SelectObject(mem, oldfont);
  DeleteObject(font);
  SelectObject(mem, old);
  DeleteObject(bm);
  DeleteDC(mem);
}
void CXEngine::XInit() {
  // Keep framebuffer dimensions in physical pixels on scaled Windows desktops.
  SetProcessDPIAware();
  CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr,
                              CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wic))))
    throw std::runtime_error("Windows image decoder unavailable");
  WNDCLASSA wc{};
  wc.style = CS_OWNDC;
  wc.lpfnWndProc = proc;
  wc.hInstance = GetModuleHandleA(nullptr);
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wc.lpszClassName = "DeadBabiesXNative";
  RegisterClassA(&wc);
  RECT rect{0, 0, 960, 720};
  AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
  window = CreateWindowA(wc.lpszClassName, "Dead Babies X Native",
                         WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT,
                         CW_USEDEFAULT, rect.right - rect.left,
                         rect.bottom - rect.top, nullptr, nullptr, wc.hInstance,
                         nullptr);
  if (!window)
    throw std::runtime_error("Cannot create window");
  // Borderless fullscreen uses the desktop resolution without changing its
  // display mode. At 1920x1080 the 4:3 viewport is 1440x1080 with side bars.
  if (!getenv("DBX_WINDOWED"))
    fullscreen();
  dc = GetDC(window);
  PIXELFORMATDESCRIPTOR pf{};
  pf.nSize = sizeof(pf);
  pf.nVersion = 1;
  pf.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pf.iPixelType = PFD_TYPE_RGBA;
  pf.cColorBits = 32;
  int pixel = ChoosePixelFormat(dc, &pf);
  if (!pixel || !SetPixelFormat(dc, pixel, &pf))
    throw std::runtime_error("Cannot set GPU pixel format");
  rc = wglCreateContext(dc);
  if (!rc || !wglMakeCurrent(dc, rc))
    throw std::runtime_error("Cannot initialize OpenGL");
  fprintf(stderr, "Renderer: %s / %s\n", glGetString(GL_VENDOR),
          glGetString(GL_RENDERER));
  auto swap = (BOOL(WINAPI *)(int))wglGetProcAddress("wglSwapIntervalEXT");
  if (swap)
    swap(0);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  applyViewport();
  makeFont();
  initAudio();
  timeBeginPeriod(1);
  QueryPerformanceFrequency(&frequency);
  QueryPerformanceCounter(&lastFrame);
  m_Millisecs = lastFrame;
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-20, 660, 500, -20, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}
void CXEngine::RequestExit() { running = false; }
bool CXEngine::TakeMenuBack() {
  bool result = menuBack;
  menuBack = false;
  return result;
}
void CXEngine::ClearInputEdges() {
  menuBack = false;
  for (auto &p : GamePad) {
    p.wPressedButtons = 0;
    std::fill(std::begin(p.bPressedAnalogButtons),
              std::end(p.bPressedAnalogButtons), 0);
  }
}
void CXEngine::SetAudioPaused(bool paused) {
  {
    std::lock_guard<std::mutex> lock(audioMutex);
    audioPaused = paused;
  }
  if (paused)
    for (DWORD i = 0; i < 4; i++) {
      XINPUT_VIBRATION v{};
      XInputSetState(i, &v);
    }
}
bool CXEngine::Running() const { return running; }
void CXEngine::XUpdate() {
  SwapBuffers(dc);
  frames++;
  LARGE_INTEGER now;
  QueryPerformanceCounter(&now);
  double elapsed =
      double(now.QuadPart - lastFrame.QuadPart) / frequency.QuadPart;
  double target = 1.0 / 60.0;
  if (!getenv("DBX_UNCAPPED")) {
    while (elapsed < target) {
      DWORD ms = (DWORD)((target - elapsed) * 1000);
      if (ms > 1)
        Sleep(ms - 1);
      else
        std::this_thread::yield();
      QueryPerformanceCounter(&now);
      elapsed = double(now.QuadPart - lastFrame.QuadPart) / frequency.QuadPart;
    }
  }
  lastFrame = now;
  m_Millisecs = now;
  seconds += elapsed;
  m_Ticks += m_GameSpeed;
  m_SoundsPerFrame = 16;
  MSG msg;
  while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessageA(&msg);
  }
  for (int i = 0; i < 4; i++) {
    auto &p = GamePad[i];
    XINPUT_STATE state{};
    WORD buttons = 0;
    BYTE analog[8]{};
    float lx = 0, ly = 0, rx = 0, ry = 0;
    bool connected = XInputGetState(i, &state) == ERROR_SUCCESS;
    if (connected) {
      auto &g = state.Gamepad;
      buttons = g.wButtons;
      WORD masks[] = {XINPUT_GAMEPAD_A,
                      XINPUT_GAMEPAD_B,
                      XINPUT_GAMEPAD_X,
                      XINPUT_GAMEPAD_Y,
                      XINPUT_GAMEPAD_RIGHT_SHOULDER,
                      XINPUT_GAMEPAD_LEFT_SHOULDER};
      for (int j = 0; j < 6; j++)
        analog[j] = (g.wButtons & masks[j]) ? 255 : 0;
      analog[6] = g.bLeftTrigger;
      analog[7] = g.bRightTrigger;
      auto axis = [](SHORT v) {
        float f = std::clamp(v / 32767.0f, -1.0f, 1.0f);
        return std::abs(f) < 0.30f
                   ? 0.0f
                   : std::copysign((std::abs(f) - 0.30f) / 0.70f, f);
      };
      lx = axis(g.sThumbLX);
      ly = axis(g.sThumbLY);
      rx = axis(g.sThumbRX);
      ry = axis(g.sThumbRY);
    }
    if (i == 0) {
      if (key(VK_RETURN) && !key(VK_MENU))
        buttons |= 0x10;
      if (key('A'))
        lx = -1;
      if (key('D'))
        lx = 1;
      if (key('W'))
        ly = 1;
      if (key('S'))
        ly = -1;
      if (key(VK_LEFT))
        rx = -1;
      if (key(VK_RIGHT))
        rx = 1;
      if (key(VK_UP))
        ry = 1;
      if (key(VK_DOWN))
        ry = -1;
      if (key(VK_SPACE) || key(VK_LBUTTON))
        analog[7] = 255;
      if (key(VK_LSHIFT) || key(VK_RBUTTON))
        analog[6] = 255;
      if (key('Q'))
        analog[4] = 255;
      if (key('E'))
        analog[5] = 255;
      connected = true;
    }
    const char *test = getenv("DBX_STRESS");
    if (test && i == 0) {
      connected = true;
      analog[7] = 255;
      rx = std::sin(seconds) * 0.6f;
      ry = 0.8f;
      lx = std::sin(seconds * 0.4) * 0.5f;
    }
    p.hDevice = connected ? (HANDLE)(uintptr_t)(i + 1) : nullptr;
    p.wPressedButtons = (buttons ^ p.wLastButtons) & buttons;
    p.wLastButtons = p.wButtons = buttons;
    for (int j = 0; j < 8; j++) {
      p.bPressedAnalogButtons[j] = analog[j] > 75 && !p.bLastAnalogButtons[j];
      p.bLastAnalogButtons[j] = analog[j] > 75;
      p.bAnalogButtons[j] = analog[j];
    }
    p.fX1 = lx;
    p.fY1 = ly;
    p.fX2 = rx;
    p.fY2 = ry;
  }
  static double report = 0;
  static unsigned long long previous = 0;
  if (seconds - report >= 2) {
    double fps = (frames - previous) / (seconds - report);
    char title[128];
    snprintf(title, sizeof(title), "Dead Babies X Native | %.1f FPS", fps);
    SetWindowTextA(window, title);
    fprintf(stderr, "FPS %.1f frames %llu elapsed %.1f\n", fps, frames,
            seconds);
    previous = frames;
    report = seconds;
  }
  const char *limit = getenv("DBX_TEST_SECONDS");
  if (limit && seconds >= atof(limit))
    running = false;
  applyViewport();
  glClear(GL_COLOR_BUFFER_BIT);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-20, 660, 500, -20, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}
static int artOverride = -1;
const char *NativeArtLabel(int mode) {
  return mode == 6 ? "ALPINE DUSK" : mode == 5 ? "CARTOON" : mode == 4 ? "CEL SHADED" : mode == 3 ? "SUPER SUPER HD" : mode == 2 ? "SUPER HD" : mode == 1 ? "HD" : "ORIGINAL";
}
static const char *NativeSuperSuperPack() {
  if (GetFileAttributesA("game/cinematic/manifest.json") != INVALID_FILE_ATTRIBUTES) return "cinematic";
  if (GetFileAttributesA("game/supersuperhd/manifest.json") != INVALID_FILE_ATTRIBUTES) return "supersuperhd";
  return nullptr;
}
bool NativeHasArtMode(int mode) {
  if (mode == 3) return NativeSuperSuperPack() != nullptr;
  if (mode == 0 || mode == 4 || mode == 5) return true;
  const char *manifest = mode == 1 ? "game/hd/manifest.json" :
                         mode == 2 ? "game/superhd/manifest.json" :
                         mode == 3 ? "game/supersuperhd/manifest.json" :
                         mode == 6 ? "game/alpine/manifest.json" : nullptr;
  return manifest && GetFileAttributesA(manifest) != INVALID_FILE_ATTRIBUTES;
}
bool NativeSetArtMode(int mode) {
  if (!NativeHasArtMode(mode)) return false;
  artOverride = mode;
  return true;
}
bool NativeHasHDArt() { return NativeHasArtMode(1); }
bool NativeSetHDArt(bool enabled) { return NativeSetArtMode(enabled ? 1 : 0); }
const char *NativeArtSkin() {
  if (artOverride == 0) return nullptr;
  if (artOverride == 1) return NativeHasArtMode(1) ? "hd" : nullptr;
  if (artOverride == 2) return NativeHasArtMode(2) ? "superhd" : nullptr;
  if (artOverride == 3) return NativeSuperSuperPack();
  if (artOverride == 4) return "celshaded";
  if (artOverride == 5) return "cartoon";
  if (artOverride == 6) return NativeHasArtMode(6) ? "alpine" : nullptr;
  if (getenv("DBX_ORIGINAL_ART")) return nullptr;
  if (getenv("DBX_CARTOON")) return "cartoon";
  if (getenv("DBX_CEL_SHADED")) return "celshaded";
  if (getenv("DBX_SUPER_SUPER_HD") && NativeHasArtMode(3)) return NativeSuperSuperPack();
  if (getenv("DBX_SUPER_HD") && NativeHasArtMode(2)) return "superhd";
  if (NativeHasArtMode(1)) return "hd";
  return nullptr;
}
IDirect3DTexture8 *CXEngine::LoadTexture(const char *name) {
  std::string filename = path(name);
  bool replacementArt = false;
  const char *skin = NativeArtSkin();
  if (skin) {
    std::string relative = name;
    if (relative.size() > 2 && relative[1] == ':') relative = relative.substr(3);
    for (char &c : relative) {
      if (c == '\\') c = '/';
      if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
    }
    std::vector<const char *> packs{skin};
    if (std::string(skin) == "cinematic") packs.push_back("supersuperhd");
    if (std::string(skin) == "celshaded") packs.push_back("supersuperhd");
    if (std::string(skin) == "supersuperhd" || std::string(skin) == "cinematic" || std::string(skin) == "celshaded") packs.push_back("superhd");
    if (std::string(skin) != "hd" && std::string(skin) != "cartoon") packs.push_back("hd");
    for (size_t i = 0; i < packs.size(); ++i) {
      std::string replacement = std::string("game/") + packs[i] + "/" + relative;
      if (GetFileAttributesA(replacement.c_str()) != INVALID_FILE_ATTRIBUTES) {
        filename = replacement;
        replacementArt = true;
        break;
      }
    }
  }

  const bool celShaded = skin && std::string(skin) == "celshaded";
  const bool cartoon = skin && std::string(skin) == "cartoon";
  const bool alpine = skin && std::string(skin) == "alpine";
  const std::string cacheKey = alpine ? "alpine:" + filename : cartoon ? "cartoon:" + filename : celShaded ? "celshaded:" + filename : filename;
  auto it = textures.find(cacheKey);
  if (it != textures.end())
    return it->second.get();
  IWICBitmapDecoder *decoder = nullptr;
  IWICBitmapFrameDecode *frame = nullptr;
  IWICFormatConverter *converter = nullptr;
  HRESULT hr = wic->CreateDecoderFromFilename(
      wide(filename).c_str(), nullptr, GENERIC_READ,
      WICDecodeMetadataCacheOnLoad, &decoder);
  if (SUCCEEDED(hr))
    hr = decoder->GetFrame(0, &frame);
  if (SUCCEEDED(hr))
    hr = wic->CreateFormatConverter(&converter);
  if (SUCCEEDED(hr))
    hr = converter->Initialize(frame, GUID_WICPixelFormat32bppRGBA,
                               WICBitmapDitherTypeNone, nullptr, 0,
                               WICBitmapPaletteTypeCustom);
  auto texture = std::make_unique<IDirect3DTexture8>();
  std::vector<unsigned char> pixels;
  if (SUCCEEDED(hr)) {
    frame->GetSize(&texture->width, &texture->height);
    pixels.resize(size_t(texture->width) * texture->height * 4);
    hr = converter->CopyPixels(nullptr, texture->width * 4, (UINT)pixels.size(),
                               pixels.data());
  }
  if (converter)
    converter->Release();
  if (frame)
    frame->Release();
  if (decoder)
    decoder->Release();
  if (FAILED(hr))
    throw std::runtime_error(
        "Missing or invalid texture: " + filename +
        ". Put the game folder beside the EXE."); // The original D3DX loader
                                                  // uses opaque black as its
                                                  // transparency key.
  if (!replacementArt)
    for (size_t i = 0; i < pixels.size(); i += 4)
      if (pixels[i] == 0 && pixels[i + 1] == 0 && pixels[i + 2] == 0)
        pixels[i + 3] = 0;
  if (alpine && filename.find("game/alpine/") != 0) AlpineDuskPixels(pixels, texture->width, texture->height, filename);
  if (cartoon && filename.find("game/cartoon/") != 0) NativeApplyCartoon(pixels, texture->width, texture->height);
  if (celShaded) NativeApplyCelShading(pixels, texture->width, texture->height);
  if (replacementArt || cartoon)
    fprintf(stderr, "ART %s: %s (%ux%u)\n", skin, filename.c_str(),
            texture->width, texture->height);
  // Skip empty canvas margins at draw time, retaining UVs and sprite pivots.
  // Two transparent texels of padding preserve bilinear filtering at edges.
  unsigned left = texture->width, top = texture->height, right = 0, bottom = 0;
  for (unsigned y = 0; y < texture->height; ++y)
    for (unsigned x = 0; x < texture->width; ++x)
      if (pixels[(size_t(y) * texture->width + x) * 4 + 3]) {
        left = std::min(left, x); top = std::min(top, y);
        right = std::max(right, x + 1); bottom = std::max(bottom, y + 1);
      }
  texture->empty = right == 0;
  if (!texture->empty) {
    texture->u0 = float(left > 2 ? left - 2 : 0) / texture->width;
    texture->v0 = float(top > 2 ? top - 2 : 0) / texture->height;
    texture->u1 = float(std::min(right + 2, texture->width)) / texture->width;
    texture->v1 = float(std::min(bottom + 2, texture->height)) / texture->height;
  }
  glGenTextures(1, &texture->id);
  glBindTexture(GL_TEXTURE_2D, texture->id);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  auto *result = texture.get();
  textures[cacheKey] = std::move(texture);
  return result;
}
void CXEngine::SetMaterial(float r, float g, float b, float a) {
  glColor4f(1, 1, 1, std::clamp(a, 0.f, 1.f));
}
void CXEngine::SetSpriteMode(int mode) {
  glBlendFunc(GL_SRC_ALPHA,
              mode == SM_ALPHA_ADD ? GL_ONE : GL_ONE_MINUS_SRC_ALPHA);
}
void CXEngine::SetCameraPos(float x, float y) {
  cameraX = x;
  cameraY = -y;
}
static void spriteQuad(const IDirect3DTexture8 *t, float x0, float y0,
                       float x1, float y1) {
  float left = x0 + (x1 - x0) * t->u0, right = x0 + (x1 - x0) * t->u1;
  float top = y0 + (y1 - y0) * t->v0, bottom = y0 + (y1 - y0) * t->v1;
  glBegin(GL_TRIANGLE_STRIP);
  glTexCoord2f(t->u0, t->v0); glVertex2f(left, top);
  glTexCoord2f(t->u1, t->v0); glVertex2f(right, top);
  glTexCoord2f(t->u0, t->v1); glVertex2f(left, bottom);
  glTexCoord2f(t->u1, t->v1); glVertex2f(right, bottom);
  glEnd();
}
void CXEngine::DrawSprite(IDirect3DTexture8 *t, float x, float y, float w,
                          float h, float rot) {
  if (!t || t->empty)
    return;
  glBindTexture(GL_TEXTURE_2D, t->id);
  glPushMatrix();
  glTranslatef(x + cameraX, y + cameraY, 0);
  glRotatef(-rot, 0, 0, 1);
  spriteQuad(t, -w / 2, -h / 2, w / 2, h / 2);
  glPopMatrix();
}
void CXEngine::DrawSpriteOC(IDirect3DTexture8 *t, float x, float y, float w,
                            float h, float rot) {
  if (!t || t->empty)
    return;
  glBindTexture(GL_TEXTURE_2D, t->id);
  glPushMatrix();
  glTranslatef(x + cameraX, y + cameraY, 0);
  // Original offset-centre path: offset, rotate, scale, translate.
  // Scaling before rotation in GL also preserves mirrored enemy animation.
  glScalef(w, h, 1);
  glRotatef(-rot, 0, 0, 1);
  spriteQuad(t, -.85f, -.15f, .15f, .85f);
  glPopMatrix();
}
void CXEngine::DrawTracer(IDirect3DTexture8 *t, float x1, float y1, float x2,
                          float y2, float w) {
  if (!t)
    return;
  float theta = ATan2(x2 - x1, y2 - y1), dx = Sin(theta - 90) * w,
        dy = Cos(theta - 90) * w;
  glBindTexture(GL_TEXTURE_2D, t->id);
  glBegin(GL_TRIANGLE_STRIP);
  glTexCoord2f(0, 1);
  glVertex2f(x1 - dx + cameraX, y1 - dy + cameraY);
  glTexCoord2f(1, 1);
  glVertex2f(x1 + dx + cameraX, y1 + dy + cameraY);
  glTexCoord2f(0, 0);
  glVertex2f(x2 - dx + cameraX, y2 - dy + cameraY);
  glTexCoord2f(1, 0);
  glVertex2f(x2 + dx + cameraX, y2 + dy + cameraY);
  glEnd();
}
static void text(const char *s, float x, float y, float size, int r, int g,
                 int b) {
  GLfloat old[4];
  glGetFloatv(GL_CURRENT_COLOR, old);
  glColor4f(r / 255.f, g / 255.f, b / 255.f, 1);
  glBindTexture(GL_TEXTURE_2D, fontTexture);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  for (; *s; s++, x += size * .6f) {
    unsigned ch = (unsigned char)*s;
    if (ch < 32 || ch > 126)
      continue;
    float u = (ch % 16) / 16.f, v = (ch / 16 - 2) / 8.f;
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(u, v);
    glVertex2f(x, y);
    glTexCoord2f(u + 1 / 16.f, v);
    glVertex2f(x + size * 1.45f, y);
    glTexCoord2f(u, v + 1 / 8.f);
    glVertex2f(x, y + size * 1.45f);
    glTexCoord2f(u + 1 / 16.f, v + 1 / 8.f);
    glVertex2f(x + size * 1.45f, y + size * 1.45f);
    glEnd();
  }
  glColor4fv(old);
}
static float textWidth(const char *s, float size) {
  size_t n = strlen(s);
  return n ? ((n - 1) * .6f + 1.45f) * size : 0;
}
void CXEngine::DrawUiText(const char *s, float x, float y, float size, int r,
                          int g, int b, bool centered) {
  text(s, centered ? x - textWidth(s, size) / 2 : x, y, size, r, g, b);
}
void CXEngine::DrawHudScore(const char *s, float center, int r, int g, int b) {
  // The visible border inside the 256px HUD texture is only 153px wide.
  // Keep every glyph's full quad within a padded 140px interior, including
  // the largest signed 32-bit score, and position it inside the 37px height.
  float units = textWidth(s, 1);
  float size = units > 0 ? std::min(18.f, 140.f / units) : 18.f;
  text(s, center - textWidth(s, size) / 2, 9, size, r, g, b);
}
void CXEngine::FillRect(float x, float y, float w, float h, float r, float g,
                        float b, float a) {
  GLfloat old[4];
  glGetFloatv(GL_CURRENT_COLOR, old);
  glDisable(GL_TEXTURE_2D);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColor4f(r, g, b, a);
  glBegin(GL_QUADS);
  glVertex2f(x, y);
  glVertex2f(x + w, y);
  glVertex2f(x + w, y + h);
  glVertex2f(x, y + h);
  glEnd();
  glEnable(GL_TEXTURE_2D);
  glColor4fv(old);
}
void CXEngine::DrawText(const char *s, int x, int y, int r, int g, int b) {
  text(s, x, y, 20, r, g, b);
}
void CXEngine::DrawScoreText(const char *s, int x, int y, int r, int g, int b) {
  text(s, x, y, 14, r, g, b);
}
void CXEngine::DrawMenuText(const char *s, int x, int y, int r, int g, int b) {
  text(s, x, y, 28, r, g, b);
}
void CXEngine::EnableMotionBlur() {}
void CXEngine::DisableMotionBlur() {}
void CXEngine::RenderMotionBlur(float) {}
void CXEngine::SetGameSpeed(float speed) { m_GameSpeed = std::max(0.f, speed); }
bool CXEngine::SelectMusic(int choice) {
  if (choice < 0 || choice > 3) return false;
  const char *filename = choice == 3 ? "rock.ogg" : choice == 2 ? "acid-house.ogg" : choice == 1 ? "static-reactor.ogg" : "music3.ogg";
  if (GetFileAttributesA(filename) == INVALID_FILE_ATTRIBUTES) {
    fprintf(stderr, "Music file missing: %s\n", filename);
    return false;
  }
  CSound candidate;
  candidate.Create(filename);
  if (!candidate.sound || candidate.sound->samples.size() < 2) {
    fprintf(stderr, "Music decode failed: %s\n", filename);
    return false;
  }
  mciSendStringA("close dbxmusic", nullptr, 0, nullptr);
  {
    std::lock_guard<std::mutex> lock(audioMutex);
    musicVoiceId = NativeReplaceMusic(voices, musicVoiceId, nextVoice++,
                                     candidate.sound, .44f * musicVolume / 1000.f);
    musicChoice = choice;
    musicPaused = false;
  }
  fprintf(stderr, "MUSIC selected %s\n", NativeMusicLabel(choice));
  return true;
}
void CXEngine::PlayMusic(const char *name) {
  if (SelectMusic(musicChoice)) return;
  if (musicChoice != 0 && SelectMusic(0)) return;
  // Compatibility route for old packages without the original Ogg file.
  if (GetFileAttributesA("music3.ogg") != INVALID_FILE_ATTRIBUTES) return;
  std::string cmd = "open \"" + path(name) + "\" alias dbxmusic";
  mciSendStringA("close dbxmusic", nullptr, 0, nullptr);
  MCIERROR e = mciSendStringA(cmd.c_str(), nullptr, 0, nullptr);
  if (e)
    fprintf(stderr, "Music unavailable: MCI error %lu\n", (unsigned long)e);
  else
    mciSendStringA("play dbxmusic repeat", nullptr, 0, nullptr);
}
void CXEngine::PlayResultCue(bool winner) {
  const char *name=winner?"game/audio/winner-fanfare.wav":"game/audio/game-over.wav";
  CSound cue;cue.Create(name);
  if(!cue.sound||cue.sound->samples.empty()) {
    fprintf(stderr,"AUDIO cue unavailable: %s\n",name);return;
  }
  {
    std::lock_guard<std::mutex> lock(audioMutex);
    voices.push_back({cue.sound,0,false,nextVoice++,1.f,false,true,true});
  }
  fprintf(stderr,"AUDIO cue %s\n",winner?"WINNER FANFARE":"GAME OVER");
}
void CXEngine::BeginResultsAudio() {
  size_t stopped;
  {
    std::lock_guard<std::mutex> lock(audioMutex);
    stopped=NativeStopEffects(voices,musicVoiceId);
    audioPaused=false;
  }
  fprintf(stderr,"AUDIO results boundary: removed %zu effect voices\n",stopped);
  PlayResultCue(false);
}
void CXEngine::PauseMusic() {
  std::lock_guard<std::mutex> lock(audioMutex);
  musicPaused = !musicPaused;
  if (musicVoiceId) {
    for (auto &voice : voices) if (voice.id == musicVoiceId) voice.paused = musicPaused;
  } else {
    mciSendStringA(musicPaused ? "pause dbxmusic" : "resume dbxmusic", nullptr, 0, nullptr);
  }
}
void CXEngine::SetMusicVolume(int value) {
  std::lock_guard<std::mutex> lock(audioMutex);
  musicVolume = std::clamp(value, 0, 1000);
  for (auto &voice : voices)
    if (voice.id == musicVoiceId) voice.gain = .44f * musicVolume / 1000.f;
  if (!musicVoiceId) {
    char cmd[80];
    snprintf(cmd, sizeof(cmd), "setaudio dbxmusic volume to %d", musicVolume);
    mciSendStringA(cmd, nullptr, 0, nullptr);
  }
}
int NativeAudioSelfTest() {
  int failures = 0;
  auto check = [&](bool ok, const char *label) {
    printf("%s: %s\n", ok ? "PASS" : "FAIL", label);
    if (!ok) ++failures;
  };
  XEngine->StopAllSounds();
  XEngine->SetMusicVolume(1000);
  for (int i = 0; i < 100; ++i) XEngine->PlayMusic("d:\\data\\music\\music3.wma");
  check(voices.size() == 1 && musicVoiceId != 0,
        "100 music starts retain exactly one background voice");
  if (voices.size() != 1 || !voices[0].sound || voices[0].sound->samples.empty()) return 1;
  const auto samples = voices[0].sound;
  const size_t frameCount = samples->samples.size() / 2;
  std::array<short, 2048> output{};
  bool matches = true;
  size_t played = 0;
  // Run the actual decoded/resampled track through more than two full loops.
  const size_t blocks = (frameCount * 2 + 1023) / 1024 + 1;
  for (size_t block = 0; block < blocks; ++block) {
    NativeMixAudio(voices, output.data(), 1024, false);
    for (size_t sample = 0; sample < output.size(); ++sample) {
      float value = samples->samples[(played * 2 + sample) % samples->samples.size()] * .44f;
      short expected = static_cast<short>(std::clamp(value, -1.f, 1.f) * 32767.f);
      if (output[sample] != expected) matches = false;
    }
    played += 1024;
  }
  check(matches, "two full song loops preserve samples, stereo and playback position");
  printf("Simulated %.1f seconds of actual music samples\n", played / 48000.0);
  const size_t beforePause = voices[0].pos;
  XEngine->PauseMusic();
  NativeMixAudio(voices, output.data(), 1024, false);
  check(std::all_of(output.begin(), output.end(), [](short v) { return v == 0; }) &&
            voices[0].pos == beforePause, "music pause is silent and freezes the track position");
  XEngine->PauseMusic();
  NativeMixAudio(voices, output.data(), 1024, false);
  check(voices[0].pos != beforePause, "music resumes from its saved position");
  XEngine->SetMusicVolume(250);
  check(std::abs(voices[0].gain - .11f) < .00001f, "Ogg music volume controls the background voice");
  XEngine->SetMusicVolume(1000);
  XEngine->StopAllSounds();
  XEngine->PlayMusic("d:\\data\\music\\music3.wma");
  check(voices.size() == 1 && voices[0].pos == 0, "round reset restarts a single clean music voice");
  XEngine->StopAllSounds();
  check(XEngine->SelectMusic(1) && NativeMusicChoice() == 1 && voices.size() == 1,
        "Static Reactor selection replaces the original music");
  check(voices.size() == 1 && voices[0].sound->samples.size() == 48000 * 20 * 2,
        "Static Reactor decodes to an exact 20-second stereo loop");
  XEngine->SetAudioPaused(true);
  NativeMixAudio(voices, output.data(), 1024, audioPaused);
  check(voices.size() == 1 && voices[0].pos == 2048,
        "music preview advances while game effects are paused");
  XEngine->SetAudioPaused(false);
  XEngine->StopAllSounds();
  XEngine->PlayMusic("d:\\data\\music\\music3.wma");
  check(NativeMusicChoice() == 1 && voices.size() == 1 &&
            voices[0].sound->samples.size() == 48000 * 20 * 2,
        "selected track survives clearing voices and restarting a round");
  check(XEngine->SelectMusic(0) && NativeMusicChoice() == 0 && voices.size() == 1,
        "Original music can be restored without stacking voices");
  const auto beforeInvalid = musicVoiceId;
  check(!XEngine->SelectMusic(4) && NativeMusicChoice() == 0 &&
            musicVoiceId == beforeInvalid && voices.size() == 1,
        "invalid track selection preserves current playback");
  XEngine->StopAllSounds();
  check(XEngine->SelectMusic(2) && NativeMusicChoice()==2 && voices.size()==1 &&
            voices[0].sound->samples.size()==48000*20*2,"Acid House decodes and replaces the current track");
  check(XEngine->SelectMusic(3) && NativeMusicChoice()==3 && voices.size()==1 &&
            voices[0].sound->samples.size()==48000*20*2,"Rock decodes and replaces the current track");
  uint64_t preservedMusic=musicVoiceId;
  auto loopSound=voices[0].sound;
  voices.push_back({loopSound,0,true,nextVoice++});
  voices.push_back({loopSound,0,true,nextVoice++});
  XEngine->BeginResultsAudio();
  check(musicVoiceId==preservedMusic && voices.size()==2 &&
            std::count_if(voices.begin(),voices.end(),[](const auto &v){return v.loop;})==1,
        "game-over boundary stops looping effects, preserves music and queues one announcement");
  XEngine->PlayResultCue(true);
  check(voices.size()==3 && !voices.back().loop,"winner fanfare is a single nonlooping voice");
  NativeMixAudio(voices,output.data(),1024,false);
  for(int i=0;i<200;++i)NativeMixAudio(voices,output.data(),1024,false);
  check(voices.size()==1 && voices[0].id==preservedMusic,"announcement and fanfare finish without leaving stuck effects");
  XEngine->StopAllSounds();
  printf("Audio self-test: %d failures\n", failures);
  return failures ? 1 : 0;
}

void NativeRumble(HANDLE h, PadFeedback *f) {
  if (!h || !f)
    return;
  XINPUT_VIBRATION v{f->Rumble.wLeftMotorSpeed, f->Rumble.wRightMotorSpeed};
  if (audioPaused)
    v = {0, 0};
  XInputSetState((DWORD)(uintptr_t)h - 1, &v);
}
void CXEngine::Shutdown() {
  running = false;
  for (int i = 0; i < 4; i++) {
    XINPUT_VIBRATION v{};
    XInputSetState(i, &v);
  }
  shutdownAudio();
  mciSendStringA("close dbxmusic", nullptr, 0, nullptr);
  timeEndPeriod(1);
  wglMakeCurrent(nullptr, nullptr);
  wglDeleteContext(rc);
  ReleaseDC(window, dc);
  DestroyWindow(window);
  if (wic)
    wic->Release();
  CoUninitialize();
}
float Sin(float a) { return std::sin(a * 0.017453292519943295f); }
float Cos(float a) { return std::cos(a * 0.017453292519943295f); }
float ATan2(float x, float y) { return std::atan2(x, y) * 57.29577951308232f; }
float fAbs(float a) { return std::abs(a); }
float Rand() { return float(rand()) / RAND_MAX; }
float Pow(float a, float b) { return std::pow(a, b); }
float Sqrt(float a) { return std::sqrt(a); }

COLLISIONINFO CXEngine::Collide(float X1, float Y1, float R1, float DX,
                                float DY, float X2, float Y2, float R2) {
  COLLISIONINFO MyCollisionInfo;

  D3DXVECTOR2 D1;

  D1.x = DX;
  D1.y = DY;

  float D1Length = Sqrt(D1.x * D1.x + D1.y * D1.y);

  D1.x /= D1Length;
  D1.y /= D1Length;

  D3DXVECTOR2 D2;

  D2.x = X2 - X1;
  D2.y = Y2 - Y1;

  float D2Length = Sqrt(D2.x * D2.x + D2.y * D2.y);

  D2.x /= D2Length;
  D2.y /= D2Length;

  float A = (D1.x * D2.x + D1.y * D2.y) * D2Length;
  float O = Sqrt(D2Length * D2Length - A * A);

  float DestX = X1 + DX, DestY = Y1 + DY;
  float LowX, HighX, LowY, HighY;

  if (DestX < X1) {
    LowX = DestX;
    HighX = X1;
  } else {
    LowX = X1;
    HighX = DestX;
  }

  if (DestY < Y1) {
    LowY = DestY;
    HighY = Y1;
  } else {
    LowY = Y1;
    HighY = DestY;
  }

  LowX = LowX - R1 - R2;
  HighX = HighX + R1 + R2;
  LowY = LowY - R1 - R2;
  HighY = HighY + R1 + R2;

  if (O < R1 + R2 && X2 > LowX && X2 < HighX && Y2 > LowY && Y2 < HighY) {
    MyCollisionInfo.Collided = true;

    MyCollisionInfo.Pos.x = D1.x * (A - ((R1 + R2) - O)); // WRONG
    MyCollisionInfo.Pos.y = D1.y * (A - ((R1 + R2) - O)); // WRONG
  } else {
    MyCollisionInfo.Collided = false;

    MyCollisionInfo.Pos.x = D1.x * D1Length;
    MyCollisionInfo.Pos.y = D1.y * D1Length;
  }

  return MyCollisionInfo;
}

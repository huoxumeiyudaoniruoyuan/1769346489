#include <Windows.h>

#include <memory>
#include <string>

#include "AI/AIInfer.h"
#include "Hook/DX11Hook.h"
#include "UI/ControlPanel.h"

namespace {

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  default:
    return DefWindowProc(hwnd, message, wParam, lParam);
  }
}

HWND CreateMainWindow(HINSTANCE instance) {
  constexpr wchar_t kClassName[] = L"AIFrameBoostMVPWindowClass";

  WNDCLASS wc{};
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = instance;
  wc.lpszClassName = kClassName;
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

  if (!RegisterClass(&wc)) {
    return nullptr;
  }

  return CreateWindowEx(
      0,
      kClassName,
      L"AI Frame Boost MVP",
      WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      1280,
      720,
      nullptr,
      nullptr,
      instance,
      nullptr);
}

} // namespace

int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int cmdShow) {
  HWND window = CreateMainWindow(instance);
  if (!window) {
    MessageBox(nullptr, L"Failed to create main window.", L"AIFrameBoostMVP", MB_ICONERROR);
    return -1;
  }

  ShowWindow(window, cmdShow);

  ai::AIInfer infer;
  hook::DX11Hook dx11Hook;
  ui::ControlPanel panel;

  if (!dx11Hook.Initialize(&infer)) {
    MessageBox(window, L"DX11 hook initialization failed.", L"AIFrameBoostMVP", MB_ICONERROR);
    return -2;
  }

  MSG msg{};
  bool running = true;

  while (running) {
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT) {
        running = false;
      }
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    const ui::ControlState& state = panel.Render();
    if (state.requestModelReload && !state.modelPath.empty()) {
      infer.LoadModel(state.modelPath);
    }

    // In production hook path, OnFrame() is called from Present detour with
    // real backbuffer texture pointer.
    // dx11Hook.OnFrame(backBufferTexture);

    Sleep(16);
  }

  dx11Hook.Shutdown();
  return 0;
}

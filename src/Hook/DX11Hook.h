#pragma once

#include <d3d11.h>
#include <memory>

namespace ai {
class AIInfer;
}

namespace hook {

// DX11Hook owns frame interception lifecycle.
// For MVP, it provides a callable Present path that can be wired into a
// Detours/MinHook swap-chain hook in production.
class DX11Hook {
public:
  DX11Hook();
  ~DX11Hook();

  DX11Hook(const DX11Hook&) = delete;
  DX11Hook& operator=(const DX11Hook&) = delete;

  bool Initialize(ai::AIInfer* infer);
  void Shutdown();

  // Called from intercepted IDXGISwapChain::Present path.
  bool OnFrame(ID3D11Texture2D* backBufferTexture);

private:
  ai::AIInfer* infer_ = nullptr;
  bool initialized_ = false;
};

} // namespace hook

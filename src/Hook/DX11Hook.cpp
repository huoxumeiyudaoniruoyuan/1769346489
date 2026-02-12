#include "Hook/DX11Hook.h"

#include "AI/AIInfer.h"

namespace hook {

DX11Hook::DX11Hook() = default;

DX11Hook::~DX11Hook() { Shutdown(); }

bool DX11Hook::Initialize(ai::AIInfer* infer) {
  if (!infer) {
    return false;
  }

  infer_ = infer;
  initialized_ = true;

  // MVP note:
  // - Place MinHook/Detours setup here.
  // - Capture IDXGISwapChain::Present and extract backbuffer texture.
  return true;
}

void DX11Hook::Shutdown() {
  if (!initialized_) {
    return;
  }

  // MVP note:
  // - Remove function hooks here.
  initialized_ = false;
  infer_ = nullptr;
}

bool DX11Hook::OnFrame(ID3D11Texture2D* backBufferTexture) {
  if (!initialized_ || !infer_) {
    return false;
  }

  // Pass the captured frame texture to AI upscaler.
  return infer_->PredictFrame(backBufferTexture);
}

} // namespace hook

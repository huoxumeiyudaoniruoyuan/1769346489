#pragma once

#include <d3d11.h>
#include <memory>
#include <string>
#include <vector>

namespace ai {

// AIInfer encapsulates ONNX Runtime + DirectML session management.
// In the MVP, it demonstrates the complete model loading pipeline and
// keeps PredictFrame() as a safe, extensible texture processing entry point.
class AIInfer {
public:
  AIInfer();
  ~AIInfer();

  AIInfer(const AIInfer&) = delete;
  AIInfer& operator=(const AIInfer&) = delete;

  // Initializes an ONNX model and DirectML execution provider.
  // Returns true when session creation succeeds.
  bool LoadModel(const std::string& modelPath);

  // Receives a rendered frame texture from the hook layer.
  // MVP behavior: validates resources and keeps a placeholder path for full
  // GPU preprocessing/inference/postprocessing.
  bool PredictFrame(ID3D11Texture2D* frameTexture);

  bool IsModelReady() const;
  std::string LastError() const;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

} // namespace ai

#include "AI/AIInfer.h"

#include <Windows.h>
#include <wrl/client.h>

#include <onnxruntime_cxx_api.h>
#include <onnxruntime_provider_factory.h>

#include <filesystem>
#include <mutex>
#include <sstream>

namespace ai {

namespace {
std::wstring Utf8ToWide(const std::string& utf8) {
  if (utf8.empty()) {
    return {};
  }
  const int required = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
  if (required <= 1) {
    return {};
  }
  std::wstring wide(static_cast<size_t>(required - 1), L'\0');
  MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, wide.data(), required);
  return wide;
}
} // namespace

struct AIInfer::Impl {
  Ort::Env env{ORT_LOGGING_LEVEL_WARNING, "AIFrameBoostMVP"};
  Ort::SessionOptions sessionOptions;
  std::unique_ptr<Ort::Session> session;
  std::string lastError;
  std::mutex mutex;
};

AIInfer::AIInfer() : impl_(std::make_unique<Impl>()) {}

AIInfer::~AIInfer() = default;

bool AIInfer::LoadModel(const std::string& modelPath) {
  std::lock_guard<std::mutex> lock(impl_->mutex);
  impl_->lastError.clear();

  if (modelPath.empty()) {
    impl_->lastError = "Model path is empty.";
    return false;
  }

  if (!std::filesystem::exists(modelPath)) {
    impl_->lastError = "Model file does not exist: " + modelPath;
    return false;
  }

  try {
    impl_->sessionOptions = Ort::SessionOptions{};
    impl_->sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
    impl_->sessionOptions.SetExecutionMode(ExecutionMode::ORT_SEQUENTIAL);

    // Use DirectML so inference can run on NVIDIA/AMD GPUs via D3D12 backend.
    // device_id=0 selects default adapter. This can be exposed in UI later.
    OrtSessionOptionsAppendExecutionProvider_DML(impl_->sessionOptions, 0);

    const std::wstring widePath = Utf8ToWide(modelPath);
    impl_->session = std::make_unique<Ort::Session>(impl_->env, widePath.c_str(), impl_->sessionOptions);
  } catch (const Ort::Exception& ex) {
    impl_->lastError = std::string("ONNX Runtime failure: ") + ex.what();
    impl_->session.reset();
    return false;
  } catch (const std::exception& ex) {
    impl_->lastError = std::string("Unexpected error: ") + ex.what();
    impl_->session.reset();
    return false;
  }

  return true;
}

bool AIInfer::PredictFrame(ID3D11Texture2D* frameTexture) {
  std::lock_guard<std::mutex> lock(impl_->mutex);

  if (!impl_->session) {
    impl_->lastError = "PredictFrame called before model is loaded.";
    return false;
  }

  if (!frameTexture) {
    impl_->lastError = "PredictFrame received null texture.";
    return false;
  }

  // MVP placeholder pipeline:
  // 1) Copy/convert ID3D11Texture2D to model input tensor layout.
  // 2) Run Ort::Session::Run().
  // 3) Reconstruct/upscale output and write back to render target.
  // Current implementation validates texture metadata and returns success.
  D3D11_TEXTURE2D_DESC desc{};
  frameTexture->GetDesc(&desc);
  if (desc.Width == 0 || desc.Height == 0) {
    impl_->lastError = "Invalid frame texture dimensions.";
    return false;
  }

  return true;
}

bool AIInfer::IsModelReady() const { return impl_->session != nullptr; }

std::string AIInfer::LastError() const { return impl_->lastError; }

} // namespace ai

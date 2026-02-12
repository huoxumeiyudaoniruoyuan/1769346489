#include "UI/ControlPanel.h"

// If real Dear ImGui is available, define AI_FRAMEBOOST_ENABLE_IMGUI and
// include imgui.h before this file in your build setup.
#ifdef AI_FRAMEBOOST_ENABLE_IMGUI
#include <imgui.h>
#endif

namespace ui {

ControlPanel::ControlPanel() {
  // Default path can be overridden by user during runtime.
  state_.modelPath = "models/fsr2_style.onnx";
}

const ControlState& ControlPanel::Render() {
#ifdef AI_FRAMEBOOST_ENABLE_IMGUI
  ImGui::Begin("AI Frame Boost MVP");
  ImGui::Checkbox("Enable AI Upscale", &state_.aiEnabled);

  static char pathBuffer[512] = {};
  if (pathBuffer[0] == '\0' && !state_.modelPath.empty()) {
    strncpy_s(pathBuffer, state_.modelPath.c_str(), _TRUNCATE);
  }

  if (ImGui::InputText("Model Path", pathBuffer, sizeof(pathBuffer))) {
    state_.modelPath = pathBuffer;
  }

  if (ImGui::Button("Load Model")) {
    state_.requestModelReload = true;
  }
  ImGui::End();
#else
  // Headless fallback for build environments without Dear ImGui package.
  // Integrators should enable AI_FRAMEBOOST_ENABLE_IMGUI for real UI.
  state_.requestModelReload = false;
#endif
  return state_;
}

const ControlState& ControlPanel::State() const { return state_; }

} // namespace ui

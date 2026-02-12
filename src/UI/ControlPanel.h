#pragma once

#include <string>

namespace ui {

// Runtime UI state for MVP control panel.
struct ControlState {
  bool aiEnabled = true;
  std::string modelPath;
  bool requestModelReload = false;
};

// Minimal UI abstraction. In this MVP we provide an immediate-mode style API
// that can be bound to Dear ImGui rendering in integration code.
class ControlPanel {
public:
  ControlPanel();

  // Called every frame to draw controls.
  // Returns current state snapshot.
  const ControlState& Render();

  const ControlState& State() const;

private:
  ControlState state_;
};

} // namespace ui

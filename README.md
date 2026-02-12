# AIFrameBoostMVP (Windows)

一个可编译运行的 Windows 帧率增强工具 MVP：
- C++17
- DX11 Hook 框架（预留 DX12 扩展）
- ONNX Runtime + DirectML 推理入口（兼容 AMD / NVIDIA）
- CMake 构建，生成 `exe`

## 项目结构

```text
AIFrameBoostMVP/
├─ CMakeLists.txt
├─ README.md
└─ src/
   ├─ main.cpp
   ├─ AI/
   │  ├─ AIInfer.h
   │  └─ AIInfer.cpp
   ├─ Hook/
   │  ├─ DX11Hook.h
   │  └─ DX11Hook.cpp
   └─ UI/
      ├─ ControlPanel.h
      └─ ControlPanel.cpp
```

## 前置依赖

1. Windows 10/11
2. Visual Studio 2022 (MSVC v143)
3. CMake 3.20+
4. ONNX Runtime GPU 包（包含 DirectML provider）

> `ONNXRUNTIME_ROOT` 需要指向以下目录结构：
>
> - `${ONNXRUNTIME_ROOT}/include/onnxruntime_cxx_api.h`
> - `${ONNXRUNTIME_ROOT}/lib/onnxruntime.lib`
> - `${ONNXRUNTIME_ROOT}/lib/onnxruntime_providers_shared.lib`
> - `${ONNXRUNTIME_ROOT}/lib/onnxruntime_providers_dml.lib`

## 构建步骤

```bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64 -DONNXRUNTIME_ROOT="C:/deps/onnxruntime-win-x64-gpu"
cmake --build . --config Release
```

## 生成产物

Release 可执行文件位置：

```text
build/Release/AIFrameBoostMVP.exe
```

## Release 打包说明

1. 复制 `build/Release/AIFrameBoostMVP.exe`
2. 复制 ONNX Runtime 运行时 DLL（至少）：
   - `onnxruntime.dll`
   - `onnxruntime_providers_shared.dll`
   - `onnxruntime_providers_dml.dll`
3. 如果使用真实 ImGui，请一并带上对应依赖（通常是项目静态编译进 exe）
4. 将模型文件放到相对路径（例如 `models/*.onnx`）

建议目录：

```text
release/
├─ AIFrameBoostMVP.exe
├─ onnxruntime.dll
├─ onnxruntime_providers_shared.dll
├─ onnxruntime_providers_dml.dll
└─ models/
   └─ xxx.onnx
```

## 单文件 EXE 优化建议

严格意义上的“单文件 exe”在 ONNX Runtime 场景较难完全无外部文件，推荐以下策略：

1. **静态运行时**：本项目已启用 `/MT`，减少 VC 运行时外部依赖。
2. **使用自解压封装**：如 7-Zip SFX/NSIS，将 DLL 与模型打包进单启动器。
3. **内置模型资源**：将 ONNX 模型加密或资源化，首次启动释放到 `%LOCALAPPDATA%`。
4. **最小化 provider**：仅保留 DML 相关 provider，避免冗余 DLL。
5. **签名与压缩**：发布前使用 `signtool` 签名，并使用 `UPX`（谨慎，需验证兼容性）。

## 说明

- 当前是 MVP 架构：模块边界完整，核心 API 已就位。
- `PredictFrame(ID3D11Texture2D*)` 已打通纹理入口，后续可补 GPU 预处理与模型输入输出映射。
- DX12 Hook 作为下一阶段扩展点，建议在 `Hook/` 增加 `DX12Hook.*` 并复用 `AIInfer`。

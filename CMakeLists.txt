cmake_minimum_required(VERSION 3.20)
project(AIFrameBoostMVP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Force static runtime (/MT, /MTd) on MSVC for easier redistribution.
if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  add_compile_options(/W4 /permissive-)
endif()

if(NOT WIN32)
  message(FATAL_ERROR "AIFrameBoostMVP supports Windows 10/11 only.")
endif()

# ---- ONNX Runtime / DirectML SDK location ----------------------------------------------------
# Expected layout example:
#   ONNXRUNTIME_ROOT/
#     include/onnxruntime_cxx_api.h
#     lib/onnxruntime.lib
#     lib/onnxruntime_providers_shared.lib
#     lib/onnxruntime_providers_dml.lib
set(ONNXRUNTIME_ROOT "" CACHE PATH "Path to ONNX Runtime GPU package root")
if(NOT ONNXRUNTIME_ROOT)
  message(FATAL_ERROR "Please set -DONNXRUNTIME_ROOT=<path to ONNX Runtime GPU package>")
endif()

set(ONNXRUNTIME_INCLUDE_DIR "${ONNXRUNTIME_ROOT}/include")
set(ONNXRUNTIME_LIB_DIR "${ONNXRUNTIME_ROOT}/lib")

find_library(ONNXRUNTIME_LIB NAMES onnxruntime PATHS "${ONNXRUNTIME_LIB_DIR}" REQUIRED NO_DEFAULT_PATH)
find_library(ONNXRUNTIME_DML_LIB NAMES onnxruntime_providers_dml PATHS "${ONNXRUNTIME_LIB_DIR}" REQUIRED NO_DEFAULT_PATH)
find_library(ONNXRUNTIME_SHARED_PROVIDERS_LIB NAMES onnxruntime_providers_shared PATHS "${ONNXRUNTIME_LIB_DIR}" REQUIRED NO_DEFAULT_PATH)

# ---- App source ------------------------------------------------------------------------------
add_executable(AIFrameBoostMVP WIN32
  src/main.cpp
  src/Hook/DX11Hook.cpp
  src/AI/AIInfer.cpp
  src/UI/ControlPanel.cpp
)

target_include_directories(AIFrameBoostMVP PRIVATE
  src
  "${ONNXRUNTIME_INCLUDE_DIR}"
)

# Native DirectX + Windows libs
# d3d11: graphics pipeline access and texture handling
# dxgi: swap-chain inspection
# d3dcompiler: optional shader helper for future passes
# directml: explicit DML dependency for runtime provider
# comsuppw: for _com_error formatting in diagnostics (future extension)
target_link_libraries(AIFrameBoostMVP PRIVATE
  ${ONNXRUNTIME_LIB}
  ${ONNXRUNTIME_DML_LIB}
  ${ONNXRUNTIME_SHARED_PROVIDERS_LIB}
  d3d11
  dxgi
  d3dcompiler
  directml
)

target_compile_definitions(AIFrameBoostMVP PRIVATE
  NOMINMAX
  WIN32_LEAN_AND_MEAN
  UNICODE
  _UNICODE
)

set_target_properties(AIFrameBoostMVP PROPERTIES
  OUTPUT_NAME "AIFrameBoostMVP"
)

# Helpful output message for packaging scripts.
message(STATUS "Release executable will be generated at: <build>/Release/AIFrameBoostMVP.exe")

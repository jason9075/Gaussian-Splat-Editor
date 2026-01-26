# Gaussian-Splat-Editor Agent Instructions

This repository contains an OpenGL-based Gaussian Splatting editor designed for high-performance rendering and selective manipulation of `.ply` Gaussian Splat files.

## 🛠 Environment & Build System

The project uses **Nix** for environment orchestration and **CMake** with a **Makefile** wrapper for builds.

### 🚀 Development Workflow
1.  **Enter Environment:** Use `nix develop` or `direnv` to activate the development shell.
2.  **Build:** `make build` - Executes CMake and compiles the project in the `build/` directory.
3.  **Run:** `make run` - Executes the binary `./playground.app`.
4.  **Dev Loop:** `make dev` - Combines build and run.
5.  **Clean:** `make clean` - Removes the `build/` directory and compiled binaries.

### 📦 Key Dependencies
- **Graphics:** OpenGL, GLEW, SDL2.
- **Math:** GLM (GLM_ENABLE_EXPERIMENTAL is required).
- **GUI:** Dear ImGui, ImGuiFileDialog.
- **Data:** nlohmann_json, happly (for PLY parsing).

## 📝 Code Style & Guidelines

Strictly follow existing patterns to maintain consistency.

### 📐 Formatting
- **Standard:** Based on **Google Style**.
- **Indentation:** 2 spaces.
- **Column Limit:** 120 characters.
- **Configuration:** Refer to `.clang-format` in the root directory.
- **Tool:** Use `clang-format -i <file>` before committing changes.

### 🏷 Naming Conventions
- **Classes/Structs:** `PascalCase` (e.g., `GaussianSplat`, `Shader`).
- **Methods:**
    - Event Handlers/Main Hooks: `PascalCase` (e.g., `OnRender`, `OnEvent`, `OnImGuiRender`).
    - Logic/Getters/Setters: `camelCase` (e.g., `loadPlyFile`, `getViewModelMatrix`).
- **Variables:** `camelCase` (e.g., `modelPath`, `scaleFactor`).
    - *Exception:* Math/Physics parameters may use `snake_case` (e.g., `tan_fovx`, `focal_y`).
- **Files:** `PascalCase` for source and headers (e.g., `App.cpp`, `App.hpp`).

### 🏗 Architecture & Patterns
- **Precompiled Header (PCH):** `include/OPPCH.h` contains common includes. It is included as `#include <OPPCH.h>` in implementation files.
- **Entry Point:** `Main.cpp` handles SDL and OpenGL initialization.
- **Core Logic:** The `App` class (in `src/App.cpp`) manages the main application state and render loop.
- **Memory Management:**
    - Prefer `std::unique_ptr` and `std::shared_ptr` for resource ownership.
    - Use RAII patterns for OpenGL resources (VBOs, VAOs, Shaders).
- **Error Handling:** Use `std::cerr` for error logging. Fatal initialization errors should use `exit(1)`.

### 📂 Directory Structure
- `src/`: Implementation files (.cpp).
- `include/`: Header files (.hpp, .h).
- `shaders/`: GLSL shader files.
- `external/`: Vendored libraries (ImGui).
- `assets/`: 3D models and textures.

## 🧪 Testing & Quality Assurance
- **Tests:** Currently, there are no automated unit tests. Manual verification by running the app is required. If adding logic that requires automated testing, consider adding a `tests/` directory and integrating with CMake.
- **Linting:** Ensure all code adheres to `.clang-format`.

## 🤖 Instructions for AI Agents
- **Context:** This is a Nix-based project. Never suggest `apt-get` or global `pip` installs. Update `flake.nix` if new system dependencies are required.
- **Performance:** Pay attention to memory layout and GPU-CPU data transfer efficiency, as this is a 3D rendering application.
- **GUI:** ImGui code is concentrated in `src/GUI.cpp` and `App::OnImGuiRender`. Keep UI logic separated from core rendering logic.
- **Shader Files:** Update uniform locations in C++ code if you modify shader variables in `shaders/`.

---
*Created for Gaussian-Splat-Editor by Jason Kuan.*

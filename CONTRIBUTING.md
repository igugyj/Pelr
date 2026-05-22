# Contributing to Pelr

Thank you for your interest in contributing to Pelr! This document provides guidelines and instructions for contributing to the project.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Environment](#development-environment)
- [Building the Project](#building-the-project)
- [Project Architecture](#project-architecture)
- [Coding Guidelines](#coding-guidelines)
- [Pull Request Process](#pull-request-process)
- [Reporting Issues](#reporting-issues)
- [Third-Party Dependencies](#third-party-dependencies)

## Code of Conduct

This project is governed by a standard code of conduct. Please be respectful and constructive in all interactions. Harassment, discrimination, or any form of unacceptable behavior will not be tolerated.

## Getting Started

1. Fork the repository on GitHub.
2. Clone your fork locally.
3. Set up the development environment as described in the next section.
4. Create a new branch from `dev` for your work:

   ```sh
   git checkout dev
   git pull origin dev
   git checkout -b feature/your-feature-name
   ```

5. Make your changes and commit them.
6. Push to your fork and submit a pull request.

**Important:** Always branch from and target the `dev` branch, not `master`.

## Development Environment

### Prerequisites

- **Windows 10 or 11** (this is a Windows-only application)
- **Qt 6.10.1** with MinGW 64-bit toolchain (available at `D:/Qt/6.10.1/mingw_64`)
- **CMake** (bundled with Qt: `D:/Qt/Tools/CMake_64/bin/cmake.exe`)
- **Visual Studio 2022** (C++ workload required for Cubism SDK resource generation)
- **Git**
- **Python 3.10+** (optional, for TTS server and translation services)

### Third-Party Resources

Before building, the following resources must be present:

| Resource | Source | Notes |
|---|---|---|
| `thirdParty/Core/` | [Live2D Cubism SDK Native](https://www.live2d.com/en/sdk/download/native/) | **Not included in the repository.** Download `CubismSdkForNative-5-r.5.zip` and extract the `Core` folder. Governed by the Live2D Proprietary Software License. |
| `thirdParty/Framework/` | Git submodule | Initialize with `git submodule update --init --recursive` |
| `thirdParty/glew/` | Setup script | Run `thirdParty/scripts/setup_glew_glfw.bat` |
| `thirdParty/glfw/` | Setup script | Run `thirdParty/scripts/setup_glew_glfw.bat` |
| `thirdParty/stb/` | Git submodule | Included in submodule init |
| `Resources/` | Cubism SDK Demo build | See `docs/dev-init.md` for detailed instructions |
| `Resources/voicevox_core/` | [voicevox_core 0.16.4](https://github.com/VOICEVOX/voicevox_core/releases/tag/0.16.4) | Optional; required only for Japanese TTS. Download `download-windows-x64.exe` and place the output in `Resources/voicevox_core/` |

For detailed setup instructions, refer to [docs/dev-init.md](docs/dev-init.md).

## Building the Project

### Configure

```sh
D:/Qt/Tools/CMake_64/bin/cmake.exe -S . -B build -G "MinGW Makefiles"
```

### Build

```sh
D:/Qt/Tools/CMake_64/bin/cmake.exe --build build --config Debug
```

### Adding New Source Files

Source files are collected via `file(GLOB ...)` in `CMakeLists.txt`. After adding new `.cpp`, `.h`, or `.hpp` files, you **must re-run cmake configure** to pick them up.

### Debug vs Release

`CMakeLists.txt` has a `DEBUG_MODE` flag:

```txt
set(DEBUG_MODE ON)   # Debug: console window visible, CONSOLE define active
set(DEBUG_MODE OFF)  # Release: no console, WIN32_EXECUTABLE
```

## Project Architecture

```
src/
├── main.cpp              # Entry point: creates GLCore + TrayIcon
├── core/
│   ├── GLCore.h/cpp      # Main OpenGL widget, mouse/timer/event handling
│   └── tray.h            # System tray icon management
├── compatLApp/           # Live2D rendering layer (overrides Cubism SDK demo classes)
│   ├── LAppView.*        # OpenGL rendering, touch input, sprites
│   ├── LAppLive2DManager.*  # Model lifecycle, hit testing, view matrix
│   ├── LAppModel.*       # Model loading, motion/expression playback
│   ├── LAppDelegate.*    # App lifecycle, GL context init, singletons
│   ├── LAppDefine.*      # Constants (view scale, hit area names, motion groups)
│   └── LAppPal.*         # Platform abstraction (logging, file I/O)
├── ui/                   # Qt .ui form files (processed by AUTOUIC)
├── ai/                   # OpenAI-compatible chat API integration
├── tts/                  # TTS backends (voicevox, xunfei, Edge TTS)
├── utils/                # Logger, weather, audio spectrum (kissfft)
└── model/                # Live2D model extensions (extra motions, file mgmt)
```

## Coding Guidelines

### General

- **Language:** C++17
- **Naming convention:** Use `PascalCase` for class names, `camelCase` for variables and functions, `UPPER_SNAKE_CASE` for constants and preprocessor macros.
- **Indentation:** Use 4 spaces per indentation level. No tabs.
- **Braces:** Opening braces on the same line (K&R style).
- **Comments:** Write comments in Chinese or English. Keep them concise and meaningful.
- **Include order:** Local headers first, then Qt headers, then standard library headers.

### Qt & OpenGL

- Use `QOpenGLWidget` as the base class for OpenGL rendering.
- All mouse coordinate handling must account for High-DPI displays: always multiply logical coordinates by `devicePixelRatioF()` before reading framebuffer pixels.
- The default framebuffer must be cleared at the start of each render frame with `glClearColor(0,0,0,0)` and `glClear(GL_COLOR_BUFFER_BIT)` to maintain transparent background.
- Use `grabFramebuffer()` instead of `glReadPixels` for pixel reading outside `paintGL`.

### Live2D Layer (compatLApp)

The files in `src/compatLApp/` shadow identically-named files in `thirdParty/CubismNativeSamples/Samples/Common/`. When modifying these files:

- Do not modify the originals in `thirdParty/` — they are excluded from the build.
- `LAppView::OnTouchesEnded(px, py)` must use its `px, py` arguments, not `_touchManager->GetX/Y()` (which can be stale after touch end).
- Hit area names are defined as `"Head"` and `"Body"` in `LAppDefine`, matching `model3.json` tag names.

### Preprocessor

`-include GL/glew.h` is forced globally on all compilation units via CMake. Do not remove this — it resolves OpenGL type definitions before Qt headers are processed.

## Pull Request Process

1. Ensure your branch is based on the latest `dev` branch.
2. Make focused, atomic commits with clear commit messages.
3. Update documentation if your changes introduce new features or modify existing behavior.
4. Ensure the project builds successfully with `cmake --build build --config Debug`.
5. Submit a pull request targeting the `dev` branch.
6. Fill out the pull request template completely, including the self-check list.
7. A maintainer will review your PR. Address any feedback promptly.

### Commit Message Style

Use conventional commit format:

```
type(scope): brief description

Optional body with details.
```

Types: `feat`, `fix`, `refactor`, `docs`, `style`, `chore`, `build`, `test`.

Examples:
- `feat(core): add mouse transparency check on window activate`
- `fix(lapp): use px,py arguments in OnTouchesEnded instead of stale touch manager state`
- `docs: update build instructions for Qt 6.10.1`

## Reporting Issues

- Use the GitHub issue tracker: [https://github.com/igugyj/Pelr/issues](https://github.com/igugyj/Pelr/issues)
- Choose the appropriate issue template (Bug Report or Feature Request).
- Provide clear, reproducible steps for bugs.
- Include relevant logs, screenshots, and system information.

## Third-Party Dependencies

| Dependency | License | Included? |
|---|---|---|
| Live2D Cubism Core | Live2D Proprietary | No — must be downloaded separately |
| Live2D Cubism Framework | Live2D Proprietary | Yes (git submodule) |
| Qt 6.10.1 | LGPL | No — system dependency |
| GLEW | Modified BSD License | Yes (downloaded by setup script) |
| GLFW | zlib/libpng | Yes (downloaded by setup script) |
| kissfft | BSD-3-Clause | Yes (git submodule) |
| voicevox_core | OSS (MIT, Apache 2.0) | No — must be downloaded separately |
| ONNX Runtime | MIT | No — bundled with voicevox_core |

### Legal Notice for Live2D Cubism Core

The Live2D Cubism Core library is governed by the Live2D Proprietary Software License. If you distribute this application, you may need a Live2D publishing license. Review the [Live2D licensing terms](https://www.live2d.com/zh-CHS/sdk/license/) for details.

---

Thank you for contributing to Pelr!

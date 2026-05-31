# Pelr — Agent Guide

## Build

```sh
# Configure (after cloning or adding source files)
D:/Qt/Tools/CMake_64/bin/cmake.exe -S . -B build -G "MinGW Makefiles"

# Build
D:/Qt/Tools/CMake_64/bin/cmake.exe --build build --config Debug
```

Qt 6.10.1 at `D:/Qt/6.10.1/mingw_64`. Windows-only (Win10/11). No tests, no lint, no typecheck.

## Architecture

- `src/main.cpp` — entry point. Creates `GLCore` (QOpenGLWidget) + `TrayIcon`.
- `src/core/` — main OpenGL widget, window management, data/config (`data.hpp`), tray, launcher.
- `src/ui/` — Qt `.ui` forms and widget classes. AUTOUIC searches here.
- `src/ai/` — OpenAI-compatible chat API calls.
- `src/tts/` — TTS backends: voicevox, xunfei, Edge TTS (via external Python server).
- `src/translation/` — Tencent/LibreTranslate API via Qt Network.
- `src/keyboard/` — real-time key press state display.
- `src/utils/` — logger, weather, audio spectrum (kissfft), license check, update/version checker, storage info (disk usage).
- `src/model/` — Live2D model extension (extra motions, file management).
- `src/compatLApp/` — shadows `CubismNativeSamples/Samples/Common/` classes via `include_directories(BEFORE)` + exclusion from GLOB.
- `src/compatSDK/` — same shadow mechanism for `CubismNativeFramework/src/Rendering/OpenGL/` headers (forces `GL/glew.h` include before Framework headers).

Source files use `GLOB` in CMakeLists.txt — **re-run cmake configure** after adding new `.cpp`/`.h`/`.hpp` files.

## Live2D Layer (compatLApp)

Six files shadow identically-named files in `thirdParty/CubismNativeSamples/Samples/Common/` (originals excluded from build):

| File | Role |
|---|---|
| `LAppView.hpp/cpp` | OpenGL rendering, touch input routing, sprite/render-target management |
| `LAppLive2DManager.hpp/cpp` | Model lifecycle, hit-test dispatching (`OnTap`), view matrix |
| `LAppModel.hpp/cpp` | Model loading, hit testing, motion/expression playback |
| `LAppDelegate.hpp/cpp` | App lifecycle, GL context init, singleton accessors |
| `LAppDefine.hpp/cpp` | Constants (view scale, hit area names, motion groups, priorities) |
| `LAppPal.hpp/cpp` | Platform abstraction (logging, file I/O) |

Key parent classes live in `thirdParty/`:
- `LAppView_Common` (in `CubismNativeSamples/Samples/Common/`)
- `LAppModel_Common` (same path)
- `CubismUserModel` (in `CubismNativeFramework/src/Model/`)
- `CubismMatrix44`, `CubismViewMatrix`, `CubismModelMatrix` (in `CubismNativeFramework/src/Math/`)

## High-DPI / Framebuffer Pitfall

`grabFramebuffer()` returns a `QImage` in **physical** pixels. Mouse event coordinates are **logical**. Always multiply by `devicePixelRatioF()`:

```cpp
QImage frame = grabFramebuffer();
qreal dpr = this->devicePixelRatioF();
QPoint physical(qRound(localPos.x() * dpr), qRound(localPos.y() * dpr));
QColor color = frame.pixelColor(physical);
```

The default framebuffer must be cleared each frame before rendering, or background pixels carry stale alpha values:

```cpp
glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
glClear(GL_COLOR_BUFFER_BIT);
```

This is at the top of `LAppView::Render()`.

## Coordinate & Event Gotchas

- `LAppView::OnTouchesEnded(px, py)` uses its `px, py` arguments, not `_touchManager->GetX/Y()` (which can be stale).
- `GLCore::mousePressEvent` must call `OnTouchesBegan()` for the touch manager to track drag start.
- `GLCore::mouseReleaseEvent` calls `handleClick()` + `OnDrag(0,0)` instead of `OnTouchesEnded()` to bypass Live2D's OnTap dispatch — click detection uses pixel-alpha threshold (alpha < 64 is transparent).
- Hit area names in `LAppDefine`: `HitAreaNameHead`, `HitAreaNameBody`. These match `model3.json` tags.

## Third-Party Constraints

- **Live2D Cubism Core** (`Live2DCubismCore.dll`) is NOT in the repo. Must be downloaded from Live2D website and placed at `thirdParty/Core/dll/windows/x86_64/`. Licensed as "expandable application" — distributors may need a Live2D publishing license.
- GLEW, GLFW, kissfft are built from source. GLEW is static.
- `voicevox_core.dll` + `voicevox_onnxruntime.dll` are copied to output as post-build steps. The `voicevox_core` library is linked directly.
- `Live2DCubismCore.dll` is also copied to output via post-build.
- `-include GL/glew.h` is forced on all compilation units to fix OpenGL type resolution before Qt headers.
- ONNX Runtime (for voicevox) is initialized at startup in `main.cpp`.
- `windeployqt` runs as post-build to deploy Qt DLLs, plugins, and translations.
- `assets/` folder and subdirectories of `Resources/` are copied to output as post-build steps.

## Debug Mode

`CMakeLists.txt`: `set(DEBUG_MODE ON)` gives a console window + `CONSOLE` preprocessor define. Set to `OFF` for Release (no console, `WIN32_EXECUTABLE`). Also controls the `.qm` translation file copy and Qt message handler install.

## Startup Sequence

1. `initFileSys()` + `initLogFile()` — before `QApplication`
2. License dialog (`CheckApplication`) — blocks until accepted
3. `VoicevoxTTS::initializeOnnxRuntime()` — failure is non-fatal (warning only)
4. `GLCore` created; shown unless `isSilentBoot` is set (tray-only mode)

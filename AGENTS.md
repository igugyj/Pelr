# Pelr — Agent Guide

## Build

```sh
# Configure (after cloning or adding source files)
D:/Qt/Tools/CMake_64/bin/cmake.exe -S . -B build -G "MinGW Makefiles"

# Build
D:/Qt/Tools/CMake_64/bin/cmake.exe --build build --config Debug
```

Qt 6.10.1 at `D:/Qt/6.10.1/mingw_64`. Windows-only (Win10/11). No tests, no lint, no typecheck.

Source files use `file(GLOB ...)` — **re-run cmake configure** after adding new `.cpp`/`.h`/`.hpp` files.

## Architecture

- `src/main.cpp` — entry point. Creates `GLCore` (QOpenGLWidget) + `TrayIcon`.
- `src/core/data.hpp` — central runtime config (`DataManager` singleton).
- `src/compatLApp/` — shadows 6 `CubismNativeSamples/Samples/Common/` classes (originals excluded from build).
- `src/compatSDK/` — shadows `CubismNativeFramework/src/Rendering/OpenGL/` headers. Forces `-include GL/glew.h` globally.
- `src/ai/` — OpenAI-compatible chat API (`llamaclient`).
- `src/tts/` — TTS dispatch + backends. Also contains **translation API clients** (libre, tencent, pyLang), not in `src/translation/`.
- `src/translation/` — only `TranslationManager` (UI language switching), not API translation.
- `src/keyboard/` — real-time key press state display.
- `src/utils/` — logger, weather, audio spectrum (`kissfft`), license check, audio decoder (`AudioDecoder`, wraps miniaudio), TTS lip sync (`TtsLipSync`), storage/process/power queries.
- `src/model/` — Live2D model extension (extra motions, file management).
- `src/ui/` — Qt `.ui` forms + manual widgets. AUTOUIC searches here.
- `scripts/` — `clean_release.py` (dry-run via `RUN = False`), `setup_glew_glfw.bat`.
- `thirdParty/` — git submodules + downloaded SDKs. See `.gitmodules`. `Live2DCubismCore.dll` NOT in repo (must download).

## FluentUIStyle (Qt Style Plugin)

FluentUIStyle is a git submodule at `thirdParty/FluentUIStyle`. It is **not** linked as a library — it is built as a standalone Qt style plugin via `ExternalProject_Add` and auto-deployed to `D:/Qt/6.10.1/mingw_64/plugins/styles/`. At runtime `app.setStyle("FluentUI3")` loads it.

- Build uses `BUILD_LIBRARY=OFF`, `BUILD_PLUGIN=ON`, `BUILD_EXAMPLE=OFF`.
- A `PATCH_COMMAND` in CMakeLists.txt strips MSVC-only `$<TARGET_PDB_FILE:...>` generator expressions for MinGW compatibility. If FluentUIStyle's root CMakeLists.txt is ever refactored, the patch script may need updating.

## TTS Providers (`data.hpp`)

| ID | Provider | Backend |
|----|----------|---------|
| 0 | Edge TTS | external Python server (Pelr_tts_tr) |
| 1 | iFlytek | external Python server (Pelr_tts_tr) |
| 2 | voicevox | local, needs Core DLL |
| 3 | OpenAI-Compatible | direct HTTP API |

## TTS Lip Sync Pipeline

`voicegenerator.hpp` emits `voiceGenerated(filePath)` → `BubbleBox::playVoice()` (`QMediaPlayer`) + `LAppModel::StartLipSync()` → `TtsLipSync::start()` decodes duration via `AudioDecoder`, drives `ParamMouthOpenY` with smoothed Perlin noise. Duration-based only, no RMS decoding.

Key files: `src/utils/TtsLipSync.hpp`, `src/utils/AudioDecoder.hpp/.cpp`, `src/compatLApp/LAppModel.cpp`.

## Live2D Layer (compatLApp)

Six files shadow identically-named files in `thirdParty/CubismNativeSamples/Samples/Common/` (originals excluded from build):

| File | Role |
|------|------|
| `LAppView.hpp/.cpp` | OpenGL rendering, touch input, sprite/render-target mgmt |
| `LAppLive2DManager.hpp/.cpp` | Model lifecycle, hit-test dispatching, view matrix |
| `LAppModel.hpp/.cpp` | Model loading, hit testing, motion/expression playback |
| `LAppDelegate.hpp/.cpp` | App lifecycle, GL context init, singleton accessors |
| `LAppDefine.hpp/.cpp` | Constants (view scale, hit areas, motion groups, priorities) |
| `LAppPal.hpp/.cpp` | Platform abstraction (logging, file I/O) |

## High-DPI / Framebuffer Pitfall

`grabFramebuffer()` returns physical pixels; mouse coordinates are logical. Always:

```cpp
qreal dpr = this->devicePixelRatioF();
QPoint physical(qRound(localPos.x() * dpr), qRound(localPos.y() * dpr));
QColor color = frame.pixelColor(physical);
```

Default framebuffer must be cleared each frame before rendering (at top of `LAppView::Render()`):

```cpp
glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
glClear(GL_COLOR_BUFFER_BIT);
```

## Coordinate & Event Gotchas

- `LAppView::OnTouchesEnded(px, py)` uses arguments, not `_touchManager->GetX/Y()` (stale after touch end).
- `GLCore::mousePressEvent` must call `OnTouchesBegan()`.
- `GLCore::mouseReleaseEvent` calls `handleClick()` + `OnDrag(0,0)`, not `OnTouchesEnded()` — click uses pixel-alpha threshold (< 64 = transparent).
- Hit area names in `LAppDefine`: `HitAreaNameHead`, `HitAreaNameBody` (match `model3.json`).

## Third-Party Constraints

- `-include GL/glew.h` is forced globally via CMake (`target_compile_options` + `PRIVATE -include GL/glew.h`). Do not remove.
- GLEW is static (`GLEW_STATIC` defined globally).
- `voicevox_core.dll`, `voicevox_onnxruntime.dll`, `Live2DCubismCore.dll` copied to output via post-build.
- `windeployqt` runs as post-build to deploy Qt DLLs.
- `assets/` folder and `Resources/*` subdirectories are copied to output via post-build.

## Debug vs Release

`CMakeLists.txt`: `set(DEBUG_MODE ON)` → console window + `CONSOLE` define. `OFF` → no console, `WIN32_EXECUTABLE`, Qt message handler installed.

## Startup Sequence

1. `initFileSys()` + `initLogFile()` — before `QApplication`
2. License dialog (`CheckApplication`) — blocks until accepted
3. `VoicevoxTTS::initializeOnnxRuntime()` — failure is non-fatal (warning)
4. `GLCore` created; shown unless `isSilentBoot` is set (tray-only)

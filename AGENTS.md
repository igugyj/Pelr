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
- `src/core/` — OpenGL widget, window management, data/config (`data.hpp`), tray, launcher.
- `src/ui/` — Qt `.ui` forms and manual widget classes. AUTOUIC searches here. Includes `systemMonitorWidget`, `DiskUsageRing`, `todoWidget`, `manageStart`, `setting`, `chat`, etc.
- `src/ai/` — OpenAI-compatible chat API (`llamaclient`).
- `src/tts/` — TTS dispatch via `voicegenerator.hpp` (switch on `provider` 0-3) + voicevox, Edge TTS, iFlytek, OpenAI-Compatible backends. Also contains translation API clients (libre, tencent, pyLang) — **not** in `src/translation/`.
- `src/translation/` — only `TranslationManager` (UI language switching), not API translation.
- `src/keyboard/` — real-time key press state display.
- `src/utils/` — logger, weather, audio spectrum (`kissfft`), license check, version checker, storage info (`storageInfo`), process memory (`systemInfo`), power status (`getpowerstatus`), audio decoder (`AudioDecoder`, wraps miniaudio), TTS lip sync driver (`TtsLipSync`).
- `src/model/` — Live2D model extension (extra motions `ExtraMotionManager`, file management `ExtraFileManager`).
- `src/compatLApp/` — shadows 6 `CubismNativeSamples/Samples/Common/` classes.
- `src/compatSDK/` — shadows `CubismNativeFramework/src/Rendering/OpenGL/` headers (forces `GL/glew.h` include).

Source files use `GLOB` — **re-run cmake configure** after adding new `.cpp`/`.h`/`.hpp` files.

## TTS Providers (`data.hpp:159-164`)

| ID | Name |
|---|---|
| 0 | OpenAI-Edge-TTS (external Python server) |
| 1 | iFlytek (external Python server) |
| 2 | voicevox (local, needs Core DLL) |
| 3 | OpenAI-Compatible (direct HTTP API) |

Edge TTS and iFlytek rely on a separate Python server: [Pelr_tts_tr](https://github.com/igugyj/Pelr_tts_tr). OpenAI-Compatible calls an arbitrary OpenAI-compatible endpoint directly.

## TTS Lip Sync Pipeline

`voicegenerator.hpp` emits `voiceGenerated(filePath)` signal → received by both `BubbleBox::playVoice()` (`QMediaPlayer`) and `LAppModel::StartLipSync()` → `TtsLipSync::start()` decodes duration via `AudioDecoder`, drives `ParamMouthOpenY` with smoothed Perlin noise during playback. No RMS decoding — purely duration-based envelope simulation.

Key files: `src/utils/TtsLipSync.hpp`, `src/utils/AudioDecoder.hpp/cpp`, `src/compatLApp/LAppModel.cpp` (`StartLipSync`/`StopLipSync`/`Update()`).

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

The default framebuffer must be cleared each frame before rendering:

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
- miniaudio is built from source (git submodule `thirdParty/miniaudio/miniaudio.c`). Used only for audio decoding in `AudioDecoder` (WAV/MP3 → PCM float); playback uses `QMediaPlayer`.
- `voicevox_core.dll` + `voicevox_onnxruntime.dll` are copied to output as post-build steps.
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

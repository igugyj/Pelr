# Pelr — Agent Guide

## Build

```sh
# Configure (after cloning or adding source files)
D:/Qt/Tools/CMake_64/bin/cmake.exe -S . -B build/Debug -G "MinGW Makefiles"

# Build
D:/Qt/Tools/CMake_64/bin/cmake.exe --build build/Debug

# Release build in a separate tree (see "Debug vs Release" below)
D:/Qt/Tools/CMake_64/bin/cmake.exe -S . -B build/Release -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
D:/Qt/Tools/CMake_64/bin/cmake.exe --build build/Release
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
- `src/live2d/` — Live2D 模型目录/资源管理。
- `src/plugins/voicevox/` — VOICEVOX runtime plugin (`local_voicevox.dll`); hosted via `src/tts/voicevoxhost.*`.
- `src/core/componentmanager.*` / `componentpaths.hpp` — optional component discovery, validation and import.
- `src/ui/` — Qt `.ui` forms + manual widgets. AUTOUIC searches here.
- `scripts/` — 发布流程 `release.py`（①`generate_notice.py` 生成 NOTICE → ②复制构建产物 → ③`clean_release.py` 清理残留）；`setup_glew_glfw(.bat|.sh)` 实际在 `thirdParty/scripts/`。
- `scripts/release_config.json` — 三脚本次共享的外部配置：路径（`paths.build_dir`/`release_dir`/`licenses_dir`）、`clean.run`（false=清理 dry-run）、`release.copy_files`（随产物复制进 Release 根的配套文件/目录）、`notice.third_party_libs` 清单。脚本内不硬编码配置。
- `.github/workflows/release.yml` — 发布 CI（见下节 `Release CI`）；`sync-to-gitee.yml` — 镜像同步。
- `licenses/` — NOTICE 许可证全文模板（`MIT.txt`、`BSD-3-Clause.txt`、`ONNX-Runtime-MIT.txt`）。
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
- `windeployqt` runs as post-build to deploy Qt DLLs.
- `assets/` folder copied to output via post-build. Source-tree `Resources/` is not required and does not exist.
- Cubism OpenGL runtime files are copied from submodules to output on Windows post-build (output `Resources/` holds sample models only): `FrameworkShaders` from `CubismNativeFramework` OpenGL `Shaders/Standard`, `SampleShaders` from `CubismNativeSamples` OpenGL `Shaders/Standard`, and sample model dirs from `CubismNativeSamples/Samples/Resources` (root demo PNGs excluded).

### Optional component deployment (Debug builds only)

`CMakeLists.txt` deploys third-party optional components **only when `CMAKE_BUILD_TYPE` is `Debug`**. A Release build ships without them; the program then starts in a "component missing" state.

| Component | Source | Destination | Guard |
|---|---|---|---|
| Live2D Cubism Core `LICENSE.md` | `thirdParty/Core/LICENSE.md` | `<out>/Live2D/` | none — git-tracked, always present |
| `Live2DCubismCore.dll` | `thirdParty/Core/dll/windows/x86_64/` | `<out>/Live2D/` | `EXISTS` — gitignored, skipped if missing |
| voicevox `c_api`, `onnxruntime`, `dict` | `thirdParty/voicevox_core/<part>/` | `<out>/voicevox_core/<part>/` | `EXISTS` per part |
| voicevox `models` (~1.6 GB) | — | — | **not auto-copied by design** — copy manually to `<out>/voicevox_core/models` |

Copy commands are `copy_directory_if_different` (incremental, never deletes), so an existing local `voicevox_core/models` survives rebuilds. Runtime paths and the plugin host are resolved by `ComponentPaths` / `ComponentManager` (see `docs/.ai/modules/components.md`).

## Debug vs Release

`DEBUG_MODE` is the master switch for debug features (console subsystem `EXE_WIN32`/`WIN32_EXECUTABLE`, `CONSOLE` define, log level). It is **derived from `CMAKE_BUILD_TYPE`** and can be overridden:

```sh
cmake -S . -B build/Debug   -G "MinGW Makefiles"   # CMAKE_BUILD_TYPE=Debug  → DEBUG_MODE=ON
cmake -S . -B build/Release -G "MinGW Makefiles"   # CMAKE_BUILD_TYPE=Release → DEBUG_MODE=OFF
cmake -S . -B build/Release -G "MinGW Makefiles" -DDEBUG_MODE=ON   # override (verbose logs on a Release build)
```

- `CMAKE_BUILD_TYPE` defaults to `Debug` when unset — the fallback is required because FluentUIStyle's `ExternalProject_Add` forwards it through `$<CONFIG>` and an empty value breaks that subproject.
- `DEBUG_MODE` is re-derived from `CMAKE_BUILD_TYPE` on every configure unless you pass `-DDEBUG_MODE=ON/OFF` again.
- `DEBUG_MODE=ON` → console window + `CONSOLE` define. `OFF` → no console, `WIN32_EXECUTABLE`, Qt message handler installed.
- MinGW must be on `PATH` (e.g. `D:/Qt/Tools/mingw1310_64/bin`) or configure fails with `CMAKE_MAKE_PROGRAM is not set`.

## Release CI

`.github/workflows/release.yml` — 16 steps，触发为 `push` tag `v*` 或 `workflow_dispatch`（`dry_run` 选项，只出 artifact 不打 tag/不发 Release）。

- **流水线**：Checkout（submodules recursive）→ 恢复 Live2D header → 下载 GLEW 2.2.0 / GLFW 3.4 → 下载 voicevox_core C API → 装 Qt 6.10.1 + MinGW → configure → build → 解析版本/notes → `release.py`（带 4 次重试）→ 校验 Release 内容 → 7z → artifact → 打 tag → `gh release create`
- **依赖不是 submodule 的**：`thirdParty/glew|glfw|voicevox_core|Core` 全被 gitignore，所以 fresh clone 必须 CI 现场拉（这也是这条流水线除「自动化」外的唯一独立价值：证明仓库可从零构建）
- **唯一必需 Secret `LIVE2D_CORE_H_B64`**：`thirdParty/Core/include/Live2DCubismCore.h` 的 base64。Core 是 Live2D 专有许可、禁止分发，故不入仓库、不进归档；它只被 `QLibrary` 运行时加载、不参与链接，构建期只需要头文件
  ```powershell
  [Convert]::ToBase64String([IO.File]::ReadAllBytes('thirdParty/Core/include/Live2DCubismCore.h'))
  ```
- **归档**：`pelr_windows-x86_64_<tag>.7z`，`actions/upload-artifact` + `gh release create` 双份
- **易错点**：configure 必须传**单值** `-DCMAKE_PREFIX_PATH`（CMakeLists L512-513 会透传给 FluentUIStyle 的 ExternalProject）；`setup_glew_glfw.bat` 含 `pause` **不能**在 CI 用

## Startup Sequence

1. `initFileSys()` + `initLogFile()` — before `QApplication`
2. License dialog (`CheckApplication`) — blocks until accepted
3. `VoicevoxTTS::initializeOnnxRuntime()` — failure is non-fatal (warning)
4. `GLCore` created; shown unless `isSilentBoot` is set (tray-only)

# Pelr - Desktop Pet with Practical Tools

[简体中文](docs/README_zh.md)

![GitHub Repo stars](https://img.shields.io/github/stars/igugyj/pelr?style=social)
![GitHub forks](https://img.shields.io/github/forks/igugyj/pelr?style=social)
![GitHub issues](https://img.shields.io/github/issues/igugyj/pelr?style=social)

![](https://img.shields.io/badge/C++-00599C?logo=c%2B%2B&logoColor=white)
![](https://img.shields.io/badge/Qt-41CD52?logo=qt&logoColor=white)
![](https://img.shields.io/badge/License-MIT-yellow)
![](https://img.shields.io/badge/Platform-Windows-0078D6?logo=windows&logoColor=white)

**Pelr** is a Live2D-based smart desktop companion that integrates AI chat, text-to-speech, quick launch, TODO management, and a customizable virtual pet experience.

> [!NOTE]
> This project is in development; functionality and stability may be limited.
>
> Non-profit open source project, free for anyone to use.
> GitHub is the only release source. SourceForge and Gitee have been discontinued.

> [!WARNING]
> This project does NOT include Live2D Cubism Core. The Core DLL is prohibited from public distribution under Live2D's proprietary license.
> You **must** download the SDK from the [Live2D website](https://www.live2d.com/en/sdk/download/native/) and place the Core directory in the project path yourself.
> Pelr qualifies as an "expandable application." If you distribute compiled binaries, you are responsible for obtaining any required Live2D publishing license.

## Features

- **Live2D Character** - Supports model3.json format for an expressive desktop companion
- **AI Chat** - OpenAI-compatible API for natural language interaction
- **Expressions & Motions** - Play built-in expressions and motions if supported by the model
- **Text-to-Speech & Lip Sync** - Built-in support for Edge TTS, iFlytek TTS, VOICEVOX, and OpenAI-Compatible TTS. The model's mouth moves automatically when playing audio.
- **TODO Manager** - Add events and receive reminders
- **Launch Manager** - Visually manage startup items; launch any file or link (forked from [QuickTray](https://github.com/Pfolg/QuickTray))
- **Keyboard Monitor** - Real-time display of key press states (forked from [KeyMonitor](https://github.com/Pfolg/KeyMonitor))
- **Music Tray** - Tray icon rotates with system audio volume (forked from [Rotating Rhythm](https://gitee.com/Pfolg/Rotating-Rhythm))
- **Weather Service** - Real-time weather via OpenWeather integration
- **System Monitor** - Real-time memory and disk usage monitoring with detailed breakdown
- **Highly Customizable** - Rich settings to suit personal preferences

## Roadmap

- [ ] ~~Hotkey support~~
- [ ] ~~Hot-reload user configuration~~
- [ ] PMX model support (planning to integrate [saba](https://github.com/benikabocha/saba))

## Preview

<details>
<summary>Click to expand</summary>
<div style="display: flex; overflow-x: auto; gap: 10px; padding: 10px; background: #f5f5f5; border-radius: 8px;">
  <img src="repo_assets/p1.png" alt="preview1" style="width: 49%; height: auto; flex-shrink: 0; border-radius: 4px; box-shadow: 0 2px 6px rgba(0,0,0,0.1);">
  <img src="repo_assets/p2.png" alt="preview2" style="width: 49%; height: auto; flex-shrink: 0; border-radius: 4px; box-shadow: 0 2px 6px rgba(0,0,0,0.1);">
  </div>
  <div style="display: flex; overflow-x: auto; gap: 10px; padding: 10px; background: #f5f5f5; border-radius: 8px;">
  <img src="repo_assets/p3.png" alt="preview2" style="width: 49%; height: auto; flex-shrink: 0; border-radius: 4px; box-shadow: 0 2px 6px rgba(0,0,0,0.1);">
  <img src="repo_assets/p4.png" alt="preview2" style="width: 49%; height: auto; flex-shrink: 0; border-radius: 4px; box-shadow: 0 2px 6px rgba(0,0,0,0.1);">
</div>
</details>

## System Requirements

- **OS**: Windows 10/11 (Windows only)
- **CPU**: Dual-core or better
- **RAM**: 4GB or more
- **Storage**: At least 6GB free
- **GPU**: OpenGL 3.0+ support

## Quick Start

### Download & Install

See [docs/index.md](docs/index.md) for usage instructions.

### TTS Backends

- **Edge TTS / iFlytek TTS** — rely on an external Python server: [Pelr_tts_tr](https://github.com/igugyj/Pelr_tts_tr). Per open source license requirements, no pre-built package is provided.
- **VOICEVOX** — local TTS engine (bundled)
- **OpenAI-Compatible TTS** — uses any OpenAI-compatible API endpoint

### Update

```sh
git fetch && git pull
```

Update dependencies according to [CMakeLists.txt](CMakeLists.txt).

### First-Time Setup

1. **Set Live2D model path** (required)
   - Configure in Settings -> Basic Settings
   - Supports model3.json format
   - Find models at: [Booth](https://booth.pm) | [Aplaybox](https://www.aplaybox.com/)

2. **Configure TTS** (optional)
   - Edge TTS is recommended (free, no configuration needed)
   - VOICEVOX and OpenAI-Compatible TTS are also available
   - Register at [iFlytek Open Platform](https://www.xfyun.cn/) if needed
   - Fill in API credentials in Settings -> TTS

3. **Set up AI service** (optional)
   - Choose any OpenAI-compatible provider

> [!CAUTION]
> Do **not** upload the contents of the `user` folder to any platform.

## Project Structure

See [docs/dev-structure.md](docs/dev-structure.md) for details.

<details>
<summary>Architecture overview</summary>

```txt
+---------------------------------------------------------------+
|                        Pelr Desktop App                       |
+---------------------------------------------------------------+
|                                                                 |
|  main.cpp (entry point)                                       |
|    |                                                            |
|    +-- core          (tray, window management, launcher)       |
|    +-- ui            (Qt widgets: settings, chat, TODO, system monitor, etc.)  |
|    +-- ai            (OpenAI-compatible chat API)              |
|    +-- tts           (TTS dispatch: voicevox/iflytek/Edge/openai-compatible)    |
|    +-- translation   (translation via Qt Network)              |
|    |       └── Tencent Translation (Qt -> Tencent Cloud API)   |
|    |       └── LibreTranslate etc. (same)                     |
|    +-- keyboard      (keyboard state monitoring)               |
|    +-- model         (Live2D model extensions, extra motions)  |
|    +-- utils         (logging, weather, network, spectrum, storage info, process memory...)  |
|    |       └── kissfft for AudioSpectrumDetector,             |
|    |            real-time system audio analysis -> tray icon   |
|    +-- compatLApp    (Live2D rendering wrapper)               |
|                                                                 |
+---------------------------------------------------------------+
                               |
                               | depends on
                               v
+---------------------------------------------------------------+
|                    Third-Party Libraries                       |
+---------------------------------------------------------------+
|                                                                 |
|  Qt 6.10.1          (GUI, network, multimedia, serial...)       |
|  Live2D Cubism Native Framework                               |
|  Live2D Cubism Core (DLL)      (not distributable)            |
|  GLEW / GLFW       (OpenGL environment)                        |
|  kissfft           (lightweight FFT for audio spectrum)        |
|  voicevox_core     (C API, local TTS engine)                  |
|  ONNX Runtime      (bundled with voicevox_core)               |
|  stb               (image loading)                             |
|  FluentUI3Style    (Qt style plugin: Fluent UI 3 look)        |
|                                                                 |
+---------------------------------------------------------------+
                               |
                               | network calls (some modules)
                               v
+---------------------------------------------------------------+
|                   External Services / Helper Processes         |
+---------------------------------------------------------------+
|                                                                 |
|   OpenAI-compatible API   (AI chat)                           |
|   OpenAI-compatible TTS   (optional TTS backend)              |
|   iFlytek Cloud TTS API  (optional TTS backend)               |
|   OpenWeather API        (weather data)                       |
|   Tencent Cloud Translation API (via Qt Network)              |
|   LibreTranslate etc.    (same)                                |
|                                                                 |
|   Python TTS server (separate process, Edge TTS & some iFlytek)|
|      ├── Edge TTS (OpenAI-compatible audio interface)          |
|      └── iFlytek TTS HTTP wrapper (some scenarios)            |
|                                                                 |
+---------------------------------------------------------------+
```

</details>

## Tech Stack

See [NOTICE](NOTICE) for third-party notices.

### C++ Core

- **Qt 6.10.1** - Cross-platform application framework
- **OpenGL** - Graphics rendering (GLEW + GLFW)
- **Live2D Cubism** - 2D animation engine (model3.json)
- **STB** - Image processing
- **VOICEVOX** - Free, medium-quality TTS engine
- **kissfft** - Real-time spectrum analysis and audio detection
- **FluentUI3Style** - Fluent UI 3 style for Qt widgets (built as plugin via ExternalProject)

### Python Toolchain (optional)

Dependencies listed at [Pelr_tts_tr/requirements.txt](https://github.com/igugyj/Pelr_tts_tr/blob/main/requirements.txt)

## Usage Guide

- **Main navigation**: Use the left sidebar to switch between modules
- **Chat**: Send messages in the chat panel or double-click the character to open the dialog
- **Launch Manager**: Manage custom launcher entries

See [docs/index.md](docs/index.md) for detailed instructions.

## Development & Build

Quick development guide at [docs/dev-dev.md](docs/dev-dev.md)

## Contributing

- [Report bugs & suggest features](https://github.com/igugyj/Pelr/issues)
- [Submit a Pull Request](https://github.com/igugyj/Pelr/pulls)
- [Project documentation](docs)

## License

Different components use different licenses:

- Live2D Cubism SDK uses a [proprietary license](https://www.live2d.com/en/sdk/download/native/)
- Qt framework uses [LGPL/GPL](https://www.qt.io/development/download)
- Other third-party libraries: see [NOTICE](NOTICE)
- Code written by the project author in `src` is licensed under MIT
- `thirdParty/LAppLive2D` and `src/compatLApp` are derived from `CubismNativeSamples`, licensed under Live2D Open Software License
- This project is governed by all of the above licenses

## Acknowledgments

- [Live2D Cubism](https://www.live2d.com/) - Cubism Native Framework & Core for 2D animation
- [Qt Framework](https://www.qt.io/) - Cross-platform C++ framework
- [GLEW](http://glew.sourceforge.net/) - OpenGL extension loading
- [GLFW](https://www.glfw.org/) - Window and OpenGL context management
- [kissfft](https://github.com/mborgerding/kissfft) - Fast Fourier transform library
- [FluentUI3Style](https://github.com/XHY-ChuJian/FluentUIStyle) - Fluent UI 3 Qt style implementation
- [miniaudio](https://github.com/mackron/miniaudio) - Single-file audio decoding library
- [stb](https://github.com/nothings/stb) - Single-header image processing library
- [VOICEVOX CORE](https://github.com/VOICEVOX/voicevox_core) - Free TTS engine
- [ONNX Runtime](https://github.com/microsoft/onnxruntime) - Cross-platform ML inference engine
- [llama.cpp](https://github.com/ggml-org/llama.cpp) - Local AI model deployment
- [openai-edge-tts](https://github.com/travisvn/openai-edge-tts) - OpenAI-compatible Edge TTS interface
- iFlytek Open Platform - High-quality TTS service
- [Hiroshiba Kazuyuki](https://github.com/Hiroshiba) - VOICEVOX founder
- Live2D Inc. - Cubism technology
- All contributors and users for their support

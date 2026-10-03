# 开发指南

本文面向开发者，覆盖环境搭建、依赖准备、构建与发布。

普通用户无需阅读本文，直接从 [Releases](https://github.com/igugyj/Pelr/releases) 下载，用法见 [文档索引](index.md)。

> 最新的修改会提交到 [dev](https://github.com/igugyj/Pelr/tree/dev) 分支，稳定的 `release` 才会 `merge` 到主分支；想体验最新功能请使用 dev 分支。
>
> 推荐使用 VSCode 搭配 Qt、CMake、C++ 插件作为默认 IDE。

---

## 前置要求

- Visual Studio Code（或其他 IDE，建议安装 CMake Tools 扩展）
- Git
- Qt 6.10.1（CMake，MinGW）
- Python 3.11（可选，仅 TTS / 翻译服务需要）

以上软件的安装方法请参考各自官方文档。

## 依赖情况

- voicevox_core 0.17.0
- CubismSdkForNative-5-r.5

其他依赖请查看 git 子模块情况；如有新提交，请不要直接拉取，应当在测试通过后再更新，不保证最新依赖能成功工作。

> 有更新？[催更本项目的懒虫作者](https://github.com/igugyj/Pelr/issues)

系统要求：至少 **6 GB** 可用存储空间。

## 下载源代码

通过 Git 克隆仓库：

```shell
git clone --depth 1 https://github.com/igugyj/Pelr.git
```

或使用 SSH：

```shell
git clone --depth 1 git@github.com:igugyj/Pelr.git
```

进入项目目录并初始化子模块：

```shell
cd Pelr
git submodule update --init --recursive
```

如需在克隆时一步到位：

```shell
git clone --depth 1 --recursive https://github.com/igugyj/Pelr.git
```

---

## 第三方库配置

构建前需确保以下资源已就位：

- `assets`、`public`（仓库内自带）
- `thirdParty/CubismNativeFramework`、`thirdParty/CubismNativeSamples`、`thirdParty/miniz`、`thirdParty/FluentUIStyle`、`thirdParty/kissfft`、`thirdParty/miniaudio` —— git submodule，随 `git submodule update --init --recursive` 就位
- `thirdParty/stb` —— 已提交在仓库内，无需配置
- `thirdParty/Core`、`thirdParty/glew`、`thirdParty/glfw`、`thirdParty/voicevox_core` —— 不入库，按下文手动准备

### Cubism Core

根据 Live2D 专有软件许可协议，本项目**不提供**该文件。

下载地址：<https://www.live2d.com/zh-CHS/sdk/download/native/>

本项目通常支持最新版本的 Cubism Core。

1. 下载 `CubismSdkForNative-5-r.5.zip`
2. 将 `Core` 文件夹放置于 `thirdParty` 目录下
3. 保留 SDK 目录的完整性，不要直接剪切

### VOICEVOX Core

日语 TTS，可选。

参考 [VoiceVox 配置指南](app-voicevox.md)：

1. 前往 [voicevox_core 0.17.0 发布页面](https://github.com/VOICEVOX/voicevox_core/releases/tag/0.17.0)
2. 下载 `download-windows-x64.exe`
3. 运行该程序，将生成的 `voicevox_core` 文件夹**直接放到 `thirdParty` 下**

推荐目录结构（整包落盘）：

- `thirdParty/voicevox_core/c_api`
- `thirdParty/voicevox_core/onnxruntime`
- `thirdParty/voicevox_core/dict`
- `thirdParty/voicevox_core/models`

构建时 CMake 会将 `dict`、`models` 拷贝到输出目录的 `voicevox_core/`；`c_api`/`onnxruntime` 整目录不拷，其中的 DLL 复制到 exe 同级。

### GLEW、GLFW、STB

运行配置脚本：

```
thirdParty/scripts/setup_glew_glfw.bat
```

脚本执行成功即表示配置完成。注意该脚本含 `pause`，不能在 CI 中使用。

---

## 资源文件

构建期从 `thirdParty` 复制到输出目录（对使用者透明，也无需安装 Visual Studio）：

**VoiceVox（仅日语 TTS，可选）：**

- 整包放置于 `thirdParty/voicevox_core/`
- 输出（仅 Debug 构建）：`voicevox_core/c_api`、`voicevox_core/onnxruntime`、`voicevox_core/dict`
- 输出（**不自动复制**）：`voicevox_core/models`（约 1.6 GB），需手动复制到 `<输出目录>/voicevox_core/models`
- DLL 位于 `voicevox_core/c_api/lib/`、`voicevox_core/onnxruntime/lib/`（随上述目录一并复制）

**Live2D Cubism Core（必需，仅 Debug 构建投放）：**

- `thirdParty/Core/LICENSE.md` → 输出 `Live2D/LICENSE.md`
- `thirdParty/Core/dll/windows/x86_64/Live2DCubismCore.dll` → 输出 `Live2D/Live2DCubismCore.dll`（该 DLL 不入库，缺失则跳过）

**Live2D 运行时资源（着色器与示例模型）：**

- `FrameworkShaders` ← `thirdParty/CubismNativeFramework/.../OpenGL/Shaders/Standard`
- `SampleShaders` ← `thirdParty/CubismNativeSamples/Samples/OpenGL/Shaders/Standard`
- `Resources/`（示例模型）← `thirdParty/CubismNativeSamples/Samples/Resources` 的模型子目录

注意：输出目录中的 `Resources/` 仅存放 Cubism 示例模型（由 submodule 自动生成），**源码树中不存在 `Resources/` 目录**。Release 构建不投放以上**可选组件**（非着色器与示例模型），需手动补齐（见 [组件安装](app-components.md)）。

资源配置完成。

---

## 编译与运行

### 配置 CMake

编辑 `.vscode\settings.json`，确保 Qt MinGW 路径指向本地安装目录：

```json
"cmake.configureArgs": [
   "-DCMAKE_PREFIX_PATH=D:/Qt/6.10.1/mingw_64" // 修改为你的6.10.1/mingw_64路径
]
```

保存该 JSON 文件后，点击 `CMakeLists.txt` 并保存，若配置正确，Visual Studio Code 应输出（或类似消息）：

```txt
[cmake] -- Configuring done (1.4s)
[cmake] -- Generating done (0.6s)
[cmake] -- Build files have been written to: D:/repos/Pelr/Pelr/build/Debug
```

<details>
<summary>预览</summary>

![alt text](assets/image-27.png)

</details>

### 构建

从 Visual Studio Code 或命令行启动构建。预期输出：

```
[build] Live2D Cubism Core LICENSE -> Live2D/ (debug only)
[build] voicevox_core/dict -> voicevox_core/ (debug only)
[build] voicevox_core/models not auto-copied by design (1.6GB); copy it manually to <out>/voicevox_core/models to enable voicevox
[build] [100%] Built target Pelr
[driver] Build completed: 00:07:28.258
[build] Build finished with exit code 0
```

退出代码为 0 表示构建成功。

源码文件通过 `file(GLOB ...)` 收集：**新增 `.cpp`/`.h`/`.hpp` 后必须重新运行 cmake configure**，否则新文件不会进入构建。

如返回非零退出代码，请检查配置步骤。确认配置正确后仍失败，可向开发者提交问题并附上完整构建日志。

### 运行

<details>
<summary>预览</summary>

![alt text](assets/image-30.png)

</details>

### Debug 与 Release

构建类型直接决定 `DEBUG_MODE`，无需手动修改 `CMakeLists.txt`：

```shell
# 独立的 Release 构建目录（推荐，与日常 Debug 构建互不干扰）
D:/Qt/Tools/CMake_64/bin/cmake.exe -S . -B build/Release -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
D:/Qt/Tools/CMake_64/bin/cmake.exe --build build/Release
```

- `CMAKE_BUILD_TYPE=Release` → `DEBUG_MODE=OFF`（无控制台、`WIN32_EXECUTABLE`、安装 Qt 消息处理器）
- `CMAKE_BUILD_TYPE=Debug`（或未指定）→ `DEBUG_MODE=ON`（控制台窗口、`CONSOLE` 宏、详细日志）
- 需要在 Release 包上开详细日志抓 bug 时，追加 `-DDEBUG_MODE=ON` 单独覆盖
- MinGW 需在 `PATH` 上（如 `D:/Qt/Tools/mingw1310_64/bin`），否则 configure 报 `CMAKE_MAKE_PROGRAM is not set`
- 注意：Release 构建**不会投放**可选组件（Live2D Cubism Core、`voicevox_core`），首次启动即为“组件缺失”状态，需手动补齐（见 [组件安装](app-components.md)）

---

## 发布

### 本地打包

构建完成后在项目根目录运行：

```shell
py scripts\release.py
```

脚本依次执行：`generate_notice.py` 生成 NOTICE → 复制构建产物到 `Release/` → `clean_release.py` 清理残留。可修改 [scripts/release_config.json](../scripts/release_config.json) 调整打包内容（路径、随包复制的文件、NOTICE 三方库清单）。

默认情况下脚本会复制资源与许可文件、清理不需要的产物，最终内容放在 `Release` 目录。`Release` 目录内即是一个完整程序，可打包分发，但需遵循本项目的 [许可约束](../README.md#license)。

### 发布 CI（GitHub Actions）

[`.github/workflows/release.yml`](../.github/workflows/release.yml) 在 **推送 `v*` 标签** 或手动 `workflow_dispatch`（`dry_run` 只出 artifact，不打标签、不发 Release）时触发，完成从零构建到发布：

1. Checkout（含递归 submodule）→ 补齐 Live2D 头文件 → 下载 GLEW / GLFW / voicevox_core / Qt + MinGW
2. configure → build → `release.py` 打包 → 校验 `Release/` 内容
3. 生成 `pelr_windows-x86_64_<tag>.7z`，上传 artifact 并 `gh release create`

唯一必需的仓库 Secret 是 `LIVE2D_CORE_H_B64`（`thirdParty/Core/include/Live2DCubismCore.h` 的 base64），因为 Core 属 Live2D 专有许可、禁止分发，构建期只需要头文件。

发版流程：先在本地用 Release 构建验证 → 推送 `git tag v<版本>` → `git push origin v<版本>`。

首个 CI 发布：`v0.8.0`（2026-10-03，当前标记为 Pre-release）。

---

## Python TTS 服务（可选）

自 `0.4.1` 起，Python TTS 服务器已从主项目分离，可按需配置。

仓库地址：<https://github.com/igugyj/Pelr_tts_tr>

### 包管理

```shell
pip install pip-review
pip-review
pip-review --auto
```

导出依赖包到 `requirements.txt`：

```shell
pip install pigar
pigar generate
```

（`pip freeze > requirements.txt` 可导出全部包，但包含大量无关依赖，不推荐。）

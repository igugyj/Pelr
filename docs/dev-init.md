# 环境搭建指南

本文档指导开发者在 Windows 10/11 系统上完成本项目的环境配置和成功运行。

本项目依赖较为复杂，请在开始前审慎评估是否满足以下要求。

---

## 前置要求

- Visual Studio Code（或其他 IDE，建议安装 CMake Tools 扩展）
- Python 3.11（可选）
- Git
- Qt 6.10.1（CMake，MinGW）

以上软件的安装方法请参考各官方文档。

## 依赖情况

- voicevox_core 0.17.0
- CubismSdkForNative-5-r.5

其他依赖请查看git子模块情况，如有新提交，请不要直接拉取，应当在测试通过后更新，不保证最新依赖能成功工作。

> 有更新？[催更本项目的懒虫作者](https://github.com/igugyj/Pelr/issues)

## 系统要求

- 至少 **6 GB** 可用存储空间（本项目所需）

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

### VoiceVox Core

参考 [VoiceVox 配置指南](app-voicevox.md)。

1. 前往 [voicevox_core 0.17.0  发布页面](https://github.com/VOICEVOX/voicevox_core/releases/tag/0.17.0)
2. 下载 `download-windows-x64.exe`
3. 运行该程序，将生成的 `voicevox_core` 文件夹**直接放到 `thirdParty` 下**

推荐目录结构（整包落盘）：

- `thirdParty/voicevox_core/c_api`
- `thirdParty/voicevox_core/onnxruntime`
- `thirdParty/voicevox_core/dict`
- `thirdParty/voicevox_core/models`

构建时 CMake 会将 `dict`、`models` 拷贝到输出目录的 `voicevox_core/`；`c_api`/`onnxruntime` 整目录不拷，其中的 DLL 复制到 exe 同级。

---

### Cubism Core

根据 Live2D 专有软件许可协议，本项目**不提供**该文件。

下载地址：<https://www.live2d.com/zh-CHS/sdk/download/native/>

本项目通常支持最新版本的 Cubism Core。

1. 下载 `CubismSdkForNative-5-r.5.zip`
2. 将 `Core` 文件夹放置于 `thirdParty` 目录下
3. 保留 SDK 目录的完整性，不要直接剪切

### GLEW、GLFW、STB

运行配置脚本：

```
thirdParty/scripts/setup_glew_glfw.bat
```

脚本执行成功即表示配置完成。

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

注意：输出目录中的 `Resources/` 仅存放 Cubism 示例模型（由 submodule 自动生成），**源码树中不存在 `Resources/` 目录**。Release 构建不投放以上可选组件，需手动补齐（见 [组件安装](app-components.md)）。

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

保存该JSON文件后，点击`CMakeLists.txt`并保存，若配置正确，Visual Studio Code 应输出（或类似消息）：

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

如返回非零退出代码，请检查配置步骤。如确认配置正确，可向开发者提交问题，并附上完整构建日志。

### 运行

<details>
<summary>预览</summary>

![alt text](assets/image-30.png)

</details>

## 发布

构建类型直接决定 `DEBUG_MODE`，只需在配置时指定 Release：

1. 将 VSCode `CMake 插件`的 Configure 设为 Release：

```
- Configure
   - GCC 13.1.0 x86_64-w64-mingw32
   - Release
```

2. 等价于命令行指定构建类型（**不要**再手动编辑 `CMakeLists.txt`）：

```shell
D:/Qt/Tools/CMake_64/bin/cmake.exe -S . -B build/Release -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
```

> `DEBUG_MODE` 由 `CMAKE_BUILD_TYPE` 自动推导（Release → OFF）。需要临时在 Release 包上开详细日志时追加 `-DDEBUG_MODE=ON`。

3. 构建完成后在项目根目录运行：

```shell
py scripts\release.py
```

可修改[scripts\release_config.json](../scripts/release_config.json)以调整release构建的内容。

```shell
❮cmd❯  …\Pelr  dev [  ✓] via △ v4.2.3
の py scripts\release.py
NOTICE file generated successfully: D:\repos\Pelr\Pelr\NOTICE
[release] Removing existing target: D:\repos\Pelr\Pelr\Release
[release] Copied D:\repos\Pelr\Pelr\build/Release -> D:\repos\Pelr\Pelr\Release
[release] Copied file LICENSE -> D:\repos\Pelr\Pelr\Release\LICENSE
[release] Copied file NOTICE -> D:\repos\Pelr\Pelr\Release\NOTICE
[release] Copied file README.md -> D:\repos\Pelr\Pelr\Release\README.md
[release] Copied file SECURITY.md -> D:\repos\Pelr\Pelr\Release\SECURITY.md
[release] Copied file SUPPORT.md -> D:\repos\Pelr\Pelr\Release\SUPPORT.md
[release] Copied file THANKS.md -> D:\repos\Pelr\Pelr\Release\THANKS.md
[release] Copied dir  screenshots -> D:\repos\Pelr\Pelr\Release\screenshots
[DELETE] Cleaning: D:\repos\Pelr\Pelr\Release

  [dir]  Framework_autogen  (67 B)
  [dir]  glew_autogen  (62 B)
  [dir]  glfw_autogen  (62 B)
  [dir]  kissfft_autogen  (65 B)
  [dir]  miniaudio_autogen  (67 B)
  [dir]  Pelr_autogen  (1.7 MB)
  [dir]  .qt  (4.1 KB)
  [dir]  .cmake  (420.9 KB)
  [dir]  CMakeFiles  (60.9 MB)
  [dir]  log  (704 B)
  [dir]  user  (106 B)
  [dir]  voice_files  (0 B)
  [dir]  .lupdate  (25.8 KB)
  [dir]  _deps  (26.9 MB)
  [dir]  FluentUIStylePlugin-prefix  (3.7 KB)
  [file] libFramework.a  (983.9 KB)
  [file] libglew.a  (766.2 KB)
  [file] libglfw.a  (331.2 KB)
  [file] libkissfft.a  (12.9 KB)
  [file] libminiaudio.a  (975.6 KB)
  [file] cmake_install.cmake  (1.9 KB)
  [file] CMakeCache.txt  (66.5 KB)
  [file] compile_commands.json  (119.0 KB)
  [file] Makefile  (243.2 KB)
  [file] qrc_Resource.cpp  (85.5 MB)
  [file] Resource.qrc.depends  (21.8 KB)
  [file] language_en_US.qm  (5.2 KB)
  [file] language_zh_CN.qm  (33.4 KB)

Deleted 28 items, freed 178.9 MB.
[release] done.
[release] total time: 47.93 s
```

默认情况下，脚本会复制一些资源、许可文件，清理一些不需要的产物，将最终内容放在`Release`目录。

Release目录内便是一个完整的程序，可打包、分发等，但是需要遵循本项目的[约束协议](../README.md#License)。

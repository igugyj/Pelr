# 环境搭建指南

本文档指导开发者在 Windows 10/11 系统上完成本项目的环境配置和成功运行。

本项目依赖较为复杂，请在开始前审慎评估是否满足以下要求。

---

## 前置要求

- Microsoft Visual Studio 2022（C++ 开发环境）
- Visual Studio Code（或其他 IDE，建议安装 CMake Tools 扩展）
- Python 3.11（可选）
- Git
- Qt 6.10.1（CMake）

以上软件的安装方法请参考各官方文档。

## 系统要求

- 至少 **5 GB** 可用存储空间（本项目所需）
- 约 **20 GB** Visual Studio 2022 C++ 开发环境（仅需其编译输出的资源文件，本项目不由该 IDE 开发）

## 下载源代码

通过 Git 克隆仓库：

```shell
git clone https://github.com/igugyj/Pelr.git
```

或使用 SSH：

```shell
git clone git@github.com:igugyj/Pelr.git
```

进入项目目录并初始化子模块：

```shell
cd Pelr
git submodule update --init --recursive
```

如需在克隆时一步到位：

```shell
git clone --recursive https://github.com/igugyj/Pelr.git
```

---

## 第三方库配置

### VoiceVox Core

参考 [VoiceVox 配置指南](app-voicevox.md)。

1. 前往 [voicevox_core 0.16.4 发布页面](https://github.com/VOICEVOX/voicevox_core/releases/tag/0.16.4)
2. 下载 `download-windows-x64.exe`
3. 运行该程序，将生成的 `voicevox_core` 文件夹按以下结构放置：

推荐目录结构：

- `thirdParty/voicevox_core/c_api`
- `thirdParty/voicevox_core/onnxruntime`
- `Resources/voicevox_core/dict`
- `Resources/voicevox_core/models`

也可将整个 `voicevox_core` 文件夹同时放置于 `thirdParty` 和 `Resources` 目录。

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

`Resources` 目录用于存放运行时资源。这些文件不会被编译进二进制文件，而是在构建过程中复制到输出目录。

**VoiceVox 词典目录：**
`Resources/voicevox_core/dict`

**VoiceVox 模型目录：**
`Resources/voicevox_core/models`

**Live2D 资源：**

需在 `Resources/` 目录下包含以下文件夹：

- `Resources/FrameworkShaders`
- `Resources/Resources`
- `Resources/SampleShaders`

以上文件可从 Cubism SDK 获取：

1. 进入 `CubismSdkForNative-5-r.5\Samples\OpenGL`
2. 运行 `thirdParty\scripts\setup_glew_glfw.bat` 配置第三方库（该脚本与项目 `thirdParty` 目录下的脚本相同）
3. 进入 `CubismSdkForNative-5-r.5\Samples\OpenGL\Demo\proj.win.cmake\scripts`
4. 运行 `proj_msvc2022.bat`（确保已正确安装 Visual Studio 2022）

5. 进入 `CubismSdkForNative-5-r.5\Samples\OpenGL\Demo\proj.win.cmake\build\proj_msvc2022_x64_mt`
6. 使用 Visual Studio 打开 `Demo.sln`
7. 构建解决方案（Debug 或 Release 配置均可）

<details>
<summary>预览</summary>

![alt text](assets/image-24.png)

</details>

<details>
<summary>预览</summary>

![alt text](assets/image-25.png)

</details>

<details>
<summary>预览</summary>

![alt text](assets/image-26.png)

</details>

1. 构建成功后，进入 `proj_msvc2022_x64_mt\bin\Demo\Debug`（或 `Release`）
2. 复制以下三个文件夹到项目的 `Resources` 目录：
   - `FrameworkShaders`
   - `Resources`
   - `SampleShaders`

资源配置完成。

---

## 编译与运行

### 配置 CMake

编辑 `CMakeLists.txt`，确保 Qt MinGW 路径指向本地安装目录：

```txt
set(CMAKE_PREFIX_PATH "D:/Qt/6.10.1/mingw81_64")
```

保存后，如配置正确，Visual Studio Code 应输出：

```
[cmake] -- Configuring done (3.8s)
[cmake] -- Generating done (0.9s)
[cmake] -- Build files have been written to: D:/repos/Pelr/Pelr/build
```

<details>
<summary>预览</summary>

![alt text](assets/image-27.png)

</details>

### 构建

从 Visual Studio Code 或命令行启动构建。预期输出：

```
[build] Copying Resources/voicevox_core -> output directory
[build] [100%] Built target Pelr
[driver] Build finished: 00:02:56.292
[build] Build succeeded. Exit code: 0
```

退出代码为 0 表示构建成功。

如返回非零退出代码，请检查配置步骤。如确认配置正确，可向开发者提交问题，并附上完整构建日志。

### 运行

<details>
<summary>预览</summary>

![alt text](assets/image-28.png)

</details>

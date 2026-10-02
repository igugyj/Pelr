# 开发指引

致开发者：通常最新的修改会提交到 [dev](https://github.com/igugyj/Pelr/tree/dev) 分支，只有稳定的 `release` 会 `merge` 到主分支上，想体验最新的功能请用dev分支。

推荐使用VSCode搭配Qt，CMake，C++插件作为本项目默认IDE。

## Python

自 `0.4.1` 起，Python TTS 服务器已从主项目分离，可作为可选组件进行配置。

仓库地址：<https://github.com/igugyj/Pelr_tts_tr>

## C++

构建前需确保以下资源已就位：

- `assets`
- `public`
- `thirdParty/`
  - `Core`
  - `CubismNativeFramework`
  - `CubismNativeSamples`
  - `glew`
  - `stb`
  - `FluentUIStyle`
  - `kissfft`
  - `miniaudio`
  - `voicevox_core`（整包：`c_api` / `onnxruntime` / `dict` / `models`；仅日语 TTS 需要 dict/models）

## 发布版本配置

构建类型决定 `DEBUG_MODE`，无需手动改 `CMakeLists.txt`：

```shell
# 独立的 Release 构建目录（推荐，与日常 Debug 构建互不干扰）
D:/Qt/Tools/CMake_64/bin/cmake.exe -S . -B build/Release -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
D:/Qt/Tools/CMake_64/bin/cmake.exe --build build/Release
```

- `CMAKE_BUILD_TYPE=Release` → `DEBUG_MODE=OFF`（无控制台、`WIN32_EXECUTABLE`、装 Qt 消息处理器）
- 需要在 Release 包上开详细日志抓 bug 时，追加 `-DDEBUG_MODE=ON` 单独覆盖
- 注意：Release 构建**不会投放**可选组件（Live2D Cubism Core、`voicevox_core`），首次启动即为"组件缺失"状态，需手动补齐（见 [组件安装](app-components.md)）

## Python 包管理（可选）

仓库地址：<https://github.com/igugyj/Pelr_tts_tr>

```shell
pip install pip-review
pip-review
pip-review --auto
```

导出所有包（不推荐）：

```shell
pip freeze > requirements.txt
```

导出依赖包到 `requirements.txt`：

```shell
pip install pigar
pigar generate
```

# 开发指引

致开发者：通常最新的修改会提交到 [dev](https://github.com/igugyj/Pelr/tree/dev) 分支，只有稳定的 `release` 会 `merge` 到主分支上，想体验最新的功能请用dev分支。

推荐使用VSCode搭配Qt，CMake，C++插件作为本项目默认IDE。

## Python

自 `0.4.1` 起，Python TTS 服务器已从主项目分离，可作为可选组件进行配置。

仓库地址：<https://github.com/igugyj/Pelr_tts_tr>

## C++

构建前需确保以下资源已就位：

- `FrameworkShaders`
- `assets`
- `public`
- `Resources`
- `SampleShaders`
- `thirdParty/`
  - `Core`
  - `CubismNativeFramework`
  - `glew`
  - `stb`
  - `FluentUIStyle`
  - `kissfft`
  - `miniaudio`
  - `voicevox_core`

## 发布版本配置

在打包发布前，需确保 `CMakeLists.txt` 中的以下配置正确：

```txt
set(DEBUG_MODE OFF)     # ON: Debug, OFF: Release
```

## Python 包管理

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

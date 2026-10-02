# 项目结构

## 源码目录

```
src/
├── main.cpp              # 入口：固定 CWD → 初始化文件系统/日志 → 许可检查 → GLCore + 托盘
├── core/                 # DataManager 运行时配置、GLCore(OpenGL 主窗口)、托盘、启动器
│                         # + componentmanager(组件扫描/校验/导入) + componentpaths(运行时路径)
├── compatLApp/           # Live2D 六文件封装层（影子替换 CubismNativeSamples 原版）
├── compatSDK/            # 影子替换 CubismNativeFramework OpenGL 头
├── ai/                   # OpenAI 兼容聊天（LlamaClient 单例）
├── tts/                  # TTS 调度 + 4 后端 + 翻译 API 客户端 + voicevoxhost(插件宿主)
├── translation/          # 仅 UI 语言切换（TranslationManager）
├── keyboard/             # 全局输入钩子 + 按键状态覆盖层
├── live2d/               # Live2D 模型目录/资源管理
├── model/                # Live2D 模型扩展（额外动作/表情）
├── plugins/voicevox/     # VOICEVOX 插件本体 → plugins/local_voicevox.dll
├── ui/                   # Qt .ui 表单 + 手写组件（含 componentcard 组件卡片）
└── utils/                # 日志/天气/频谱/存储/许可/口型同步等

assets/                   # 运行时资源（构建期复制到输出）
public/                   # 图标/字体等 qrc 资源
translations/             # .ts 翻译源（zh_CN / en_US）
scripts/                  # 发布三脚本 + release_config.json
licenses/                 # NOTICE 许可证模板
thirdParty/               # git submodule + 手动下载 SDK
docs/                     # 用户向文档（本目录）
docs/.ai/                 # agent 向 AI 文档
```

> 源码树**不存在 `Resources/`**。输出目录中的 `Resources/`、`FrameworkShaders`、`SampleShaders` 在构建期从 `thirdParty/` 的 submodule 复制。

更完整的模块说明见 [构建与发布](dev-dev.md)；架构分层见 `.ai` 文档的 `core/architecture.md`。

## 文件树快照

本地查看：[Pelr.html](Pelr.html)

在线预览：<https://pg25-lsae.eu.org/demos/SnapshotOfPelr/demo>

> **注意**：`Pelr.html` 是外部工具生成的一次性快照（生成于 2026-10-02），**不包含**其后新增的组件系统文件（`componentmanager.*`、`componentcard.*`、`voicevoxhost.*`、`src/plugins/`）。结构以 `src/` 实际目录为准。

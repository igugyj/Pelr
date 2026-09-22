
## v0.7.29 - 2026-09-17（unrelease）

### 新特性

* **发布流水线** — 新增 `release.py` 发布流程：生成 NOTICE → 复制构建产物 → 清理；支持外部配置，内置 MIT / BSD-3-Clause / ONNX Runtime MIT 许可模板，`clean_release.py` 与 `generate_notice.py` 改为配置驱动。 [42c1efe](https://github.com/igugyj/Pelr/commit/42c1efe53f0d493c6db10a3c0e923f4c3cc83924) [f396686](https://github.com/igugyj/Pelr/commit/f396686120cdf843947fb4ef86c2bbcf3eb5dc1a)

### 重构与优化

* **DataManager 内存缓存** — 引入 `QReadWriteLock` 与惰性加载缓存，避免每帧、每次语音重复读盘解析；写入同步更新内存缓存，`writeJsonFile` 改用 `QSaveFile` 原子写入。注意：手动修改磁盘 JSON 不再运行时生效，需重启加载。 [da7f01b](https://github.com/igugyj/Pelr/commit/da7f01b2f8e22bb3bb748f6addb0b9661d34b189)
* **GLCore 生命周期治理** — 新增析构清理无父堆成员；修复 `QFutureWatcher` 生命周期，取消并等待 `QtConcurrent` 任务，避免 `this` 捕获导致 use-after-free；单例 listener/tray 连接绑定 GLCore 上下文。 [f62a053](https://github.com/igugyj/Pelr/commit/f62a053f66bd0f39d5449b77e0e2e3b2add178da) [7ccbc26](https://github.com/igugyj/Pelr/commit/7ccbc2694baddf926eb2592a074356332431dcb2)

### 问题修复

* **H15 配置篡改导致任意代码执行** — 启动时自动静默运行 Star 类目全部程序；增加确认/签名校验，`user/` 路径不再依赖 CWD。 [94bddec](https://github.com/igugyj/Pelr/commit/94bddec55e07b3e7c51cfef931674129dd5a2625)
* **H12 WeatherManager API Key 明文 HTTP** — 改用 HTTPS，并修正空城市检查误查 API Key。 [dee40f2](https://github.com/igugyj/Pelr/commit/dee40f241dc5679d84f23d94e5d51da7faf274a6)
* **H11 launcherMenu::initMenu() 重建泄漏** — 重建前删除旧 submenu 与 `QueueTimer`。 [5202faf](https://github.com/igugyj/Pelr/commit/5202faf6ec2cfc80e32dfe1ad7a1dcdba0120f52)
* **H8 & H9** — 修复对应高危问题。 [26121e1](https://github.com/igugyj/Pelr/commit/26121e1c2e88a7fdf38b39f4d4fc13b8bcb353f4)
* **H7 LibreTranslateClient 旧 reply 误删新 reply** — 校验 `sender() == m_currentReply` 或按 reply 连接。 [c759542](https://github.com/igugyj/Pelr/commit/c759542cd93820000432c47f385b28dc05a3a181)
* **H6 VoiceGenerator pending 状态被覆盖** — 加入请求序列号/队列。 [3e80b9b](https://github.com/igugyj/Pelr/commit/3e80b9bcbdc5d13015df34f0673f736c55f47367)
* **H5 LlamaClient::m_id 并发响应串号** — 每个请求使用独立 id 并校验；补充 `llamaclient.cpp` 修复。 [9703f11](https://github.com/igugyj/Pelr/commit/9703f11251f1af5b42e6591935da180b4947c78e) [f2c4720](https://github.com/igugyj/Pelr/commit/f2c4720e37f84784b0211b316966e21a10da1d61)
* **H1/H2/H3** — H1 `ExtraMotionManager::model` 悬空；H2 `LAppModel` 析构空指针；H3 `NotificationWidget::instance()` 泄漏与跨线程创建。已分别清理指针、判空、单例持久化复用。 [b314560](https://github.com/igugyj/Pelr/commit/b314560e87e80c4736f087b41df43d313b87271b)
* **TTS / ONNX 生命周期** — 序列化 `VoicevoxTTS`，应用 `speedScale`，移除 `testSynthesis`；修复 ONNX Runtime 文件名 `QByteArray` 临时对象悬空。 [963dedc](https://github.com/igugyj/Pelr/commit/963dedc9fab9bbabec3ed1f3631e29721c70909d) [8f03b20](https://github.com/igugyj/Pelr/commit/8f03b2037379005ef197f3b1b2ea9e3587718a00)
* **核心稳定性** — 修复聊天滚动区域内容控件双重删除；修复 `signMenuData` 不可用；修复日志乱码与日志级别设置。 [bbe5b87](https://github.com/igugyj/Pelr/commit/bbe5b87cd5d1e0247f47464ca3302286d2250e7b) [94543a3](https://github.com/igugyj/Pelr/commit/94543a3baed1c3f8ac8e4970a58c59eaaae17dfd) [3b2bf73](https://github.com/igugyj/Pelr/commit/3b2bf734fc3e4350e7080f67630e3566011f5f19) [47eefed](https://github.com/igugyj/Pelr/commit/47eefedb1d53bc5579c42098084bf8f53aded343)
* **UI 与交互** — 修复 `BubbleBox` 思考文本逻辑；`todoNotify` 逻辑；日期时间选择对话框时间按钮固定宽度；`NotificationWidget` 文本不更新。 [cbe714e](https://github.com/igugyj/Pelr/commit/cbe714ecabee3973689f3c6f442f84feff56631f) [ba2ac5b](https://github.com/igugyj/Pelr/commit/ba2ac5b2f27247b48473c26b0b5559ceab45694a) [afcc829](https://github.com/igugyj/Pelr/commit/afcc829f6cbf3e2af9af896a6d77939a0bc4aeb9) [715607c](https://github.com/igugyj/Pelr/commit/715607cee691ad4e6e69d7189c07cef93e91e290)

### 文档与仓库

* **第三方依赖** — 更新 `thirdParty/kissfft`。 [04e2948](https://github.com/igugyj/Pelr/commit/04e29487992a654b38a3c7819a4a0f9fabe66bb1)
* **仓库整理** — `repo_assets` 移动为 `screenshots`。 [82f6ae5](https://github.com/igugyj/Pelr/commit/82f6ae5cf6bfaead840bdf7c8986c987872174ef)

### 其他

* 本次不发布，仅记录变更日志。
* 版本号：v0.7.29。

## v0.7.4 - 2026-08-01

### What's Changed

* feat: tray setting & gif tray support by @igugyj in <https://github.com/igugyj/Pelr/pull/31>
* fix: QFileDialog blockage in using local dialog by @igugyj in <https://github.com/igugyj/Pelr/pull/32>
* **fix**: do operation in "managestart" crashed after long time running ef29c62a9104e5c7c846b042185a5cb28c107c53
* fix: window icon path cdd81f9bb171f2f69a7f01f1606813f00e531852

**Full Changelog**: <https://github.com/igugyj/Pelr/compare/v0.6.1...v0.7.4>

## v0.6.1 - 2026-06-06

### 新特性

* **多语言界面** — 界面已支持简体中文与英文动态切换
* **唇形模拟** — 播放语音时模型嘴部自动张合，模拟说话口型动态
* **语言模块正式可用** — 完整的多语言切换功能已集成

### 重构与优化

* **翻译模块重构** — 优化 Qt6 翻译配置，完善 CMake 构建流程
* **构建系统优化** — 新增 `.lupdate` 清理规则，完善构建产物清理脚本
* **文档更新** — 更新 Roadmap 标记已完成功能（快捷键、热加载配置）

### 问题修复

* **修复系统监控面板按钮尺寸异常** — 调整 `Clear` 和 `Clear All` 按钮自适应大小

### 文档与仓库

* **第三方依赖** — 新增 miniaudio 单文件音频解码库及相关鸣谢
* **开发者指引** — 完善贡献与开发文档

### 其他

* **引入新依赖** — 新增 miniaudio 子模块，需执行 `git submodule update --init --recursive`
* **版本迭代** — 自 v0.4.1 起累计 6 项功能增强与修复

## What's Changed

* release: v0.6.1 by @igugyj in <https://github.com/igugyj/Pelr/pull/29>

**Full Changelog**: <https://github.com/igugyj/Pelr/compare/v0.4.1...v0.6.1>

## v0.4.1

### 新特性

* **OpenAI-Compatible TTS** — 新增 OpenAI 兼容接口的 TTS 后端，支持自定义 endpoint、API key、模型和音色
* **系统监控面板** — 新增系统监控页面，实时显示内存占用率、磁盘占用率、进程内存详情及目录占用分析
* **随机 AI 问候** — 闲置时模型可主动发起 AI 问候对话（需配置 AI 服务）

### 重构与优化

* Qt 6 全面迁移适配
* 配置管理与部署流程重构
* 日志格式与风格统一优化
* 字体加载逻辑重构
* LLM 相关界面文案调整
* GitHub 用户名更新

### 问题修复

* 修复 voicevox TTS 缓存与模型加载问题
* 修复 Star 线程与 LookingMouseStrength 配置冲突
* 修复气泡文本格式化异常
* 修复点击坐标偏移
* 修复子进程未随父进程终止的问题

### 文档与仓库

* 更新 README（中英文）、SECURITY.md、SUPPORT.md、CONTRIBUTING.md
* 添加 Issue 与 PR 模板，完善贡献指南
* 移除不合规的许可证文件
* 添加脚本 `scripts/generate_notice.py`

### 其他

* 移除已分离的 Python TTS 服务端（移至独立仓库）
* 更新 `kissfft` 第三方库

## v20260503.14-dev

1. docs: 优化文档
2. chore: 移除旧版本 LApp 代码
3. docs: 添加 dev-init.md 文档
4. refactor: 新增两个子模块，重构第三方库目录结构
5. chore: 优化通知逻辑
6. docs: 简单大纲
7. build: 优化 CMakeLists.txt
8. fix: 修复 voicevox 测试
9. docs: 添加 CREDITS.md

## v20260503.13.5 - 2026-05-03

### 更新内容

* **文档更新：** 更新用户文档，提升使用指引的准确性与完整性。
* **TTS 设置页面：** 优化页面显示逻辑，改善界面交互体验。
* **VoiceVox 模型：** 优化模型加载时 StyleID 的载入逻辑，提高稳定性和正确性。
* **翻译功能：** 新增腾讯云翻译能力，并补充相关配置说明与文档支持。
* **翻译功能：** 基础支持 LibreTranslate。

**完整更新日志：** <https://github.com/igugyj/Pelr/compare/20260405.12.18...20260503.13.5>

---

## v20260405.12.18 - 2026-05-02

> **不兼容变更：** `configData.dat` 基本配置文件不兼容，需重新配置。

### 新特性

* **思考气泡动画：** 增加对话过程中的思考动画效果，交互更自然。
* **VoiceVox TTS 引擎支持：** 新增高质量语音合成后端，提供更多音色选择。
* **翻译后 TTS：** 可将输出文本先翻译为指定语言，再通过 TTS 朗读（Qt 界面配置，Python 端实现）。
* **自定义 AI 接口（OpenAI 兼容）：** 允许用户接入自定义 API 地址和密钥。
* **多源更新检查：** 支持从多个备用源检测新版本，提高更新可靠性。
* **通知提示音：** 新增系统通知时的可选提示音。

### 重构与优化

* **资源结构压缩：** 优化资源打包方式，显著减小构建产物体积。
* **全新 QSS 界面样式：** 引入 QSS 库重写样式表，视觉效果和一致性提升。
* **启动菜单布局优化：** 调整菜单间距与位置，减少误触概率。
* **托盘样式逻辑重构：** 规范托盘图标的激活、隐藏及消息处理逻辑。
* **设置页面 TTS 选项卡：** 重新排版 TTS 相关设置项，操作更清晰。
* **关于对话框：** 将 Markdown 文本浏览器设为自动打开外部链接。

### 问题修复

* **日志 UTF-8 支持：** 修复日志文件在 Windows 下编码乱码的问题。
* **默认气泡颜色：** 修正初始对话气泡的颜色显示异常。

### 文档与仓库

* 更新 `CREDITS.md` 与 `THANKS.md`，新增译者名单。
* 移除 Wiki，内容合并入主文档目录。
* 添加 Issue 与 PR 模板，规范社区贡献流程。
* 仓库地址从 Gitee 迁移至 GitHub，同步更新相关链接与说明。

### 依赖项变动

* 引入 `kissfft` 替代原有系统音频特征提取库，提升跨平台稳定性。

### 开发环境

* 开发工具由 CLion 替换为 VSCode，相关配置更新。

**完整更新日志：** <https://github.com/igugyj/Pelr/compare/20260219.11.17...20260405.12.18>

---

## v20260219.11.17 - 2026-04-01

> **不兼容变更：** `configData.dat` 基本配置文件不兼容，需重新配置。`ttsConfig.json` 虽兼容但建议重新配置以确保最佳体验。

### 新特性

* 音乐托盘支持颜色和符号自定义设置，优化气泡颜色逻辑。
* 新增 `openai_edge_tts` 语音合成引擎。
* 托盘菜单增加"打开用户文件夹"选项。
* TTS 连接失败时自动尝试启动服务端，并增加相关配置。

### 优化

* 控制界面适配响应式布局。
* 重构语句文件模块，提升气泡消息稳定性。
* 重构按键监听功能，优化按键提示体验。
* 窗口显示时尝试提升至最前。
* 启动线程弹窗标题及检查更新弹窗逻辑优化。

### 问题修复

* 修复启动时意外显示计划外时间气泡的问题。
* 修复按键标签中符号与键名间缺少空格的问题。
* 修正 README 中的错别字。

### 重构

* 项目重命名为 **Pelr**（原名 PLauncher）。
* 重构 src 目录结构。

**完整更新日志：** <https://gitee.com/Pfolg/Pelr/compare/20260107.10.16...20260219.11.17>

---

## 20260107.10.16

### 更新内容

1. fix: 修复日志信息中的语言不一致问题
2. feat(core): 添加批量启动功能
3. fix(setting): 添加重置设置前的确认消息
4. feat(ui): 添加 TTS 和 OpenWeather 配置结构及 JSON 读写方法
5. feat: 如无启动项则不显示菜单（首次添加需重启）
6. feat: 如系统启动超过 20 分钟则不运行 Star 线程
7. feat: 添加通知弹窗功能并替换托盘消息
8. refactor: 调整构建配置和添加资源文件复制逻辑
9. feat(ui): 添加音乐托盘图标功能
10. fix: 优化 OllamaClient 角色选择和消息处理
11. docs: 更新相关文档
12. feat(setting): 添加日志等级设置功能
13. fix(KeyLabel): 修复窗口置顶失效问题
14. feat: 添加系统托盘双击事件处理
15. feat: 自定义说话间隔和位置记录
16. fix(launcher.hpp): 优化可执行文件启动逻辑

> **不兼容变更：** `configData.dat` 基本配置文件不兼容。检查更新功能已不可用，相关 API 已更新。

**完整更新日志：** <https://gitee.com/Pfolg/Pelr/compare/ver20251118.9...20260107.10.16>

---

## ver20251118.9

### 更新内容

* fix: 窗口图标不显示
* fix(core): 修改时间语录文件路径并添加新文件
* refactor: 删除不再使用的文本文件并更新语言设置
* refactor: 移除未使用函数并添加焦点检查功能
* style(core): 添加国际化支持并调整代码格式
* style: 修改文件头注释
* feat(translations): 修改配置文件
* fix(core): 修正窗口重置位置逻辑
* fix: 修改默认文本为 "Hello World!"
* fix(log): 日志信息国际化
* fix: 优化日志信息并添加多语言支持待办
* feat(todoNotification): TODO 消息使用托盘提醒（可选）
* style/docs (thirdParty): 移除图标链接的图片标签，优化 Markdown 显示
* docs: 修改 README 中的 NOTE 格式
* docs: 修改 README 中的术语和格式
* style(ui): 移除重复包含的 data.hpp
* feat(ui): 实现菜单自动隐藏功能
* feat(setting): 添加静默启动选项

**完整更新日志：** <https://gitee.com/Pfolg/Pelr/compare/ver20251114-8...ver20251118.9>

---

## ver20251114-8

### 更新内容

* release: ver20251114-8
* chore: 删除旧部署脚本并更新版本号及忽略规则
* refactor(启动项目菜单): 使用 CustomMenu 替换 QMenu
* docs: 添加 README_en.txt 和 README_sourceforge.txt
* docs: 更新发布日志

**完整更新日志：** <https://gitee.com/Pfolg/Pelr/compare/ver20251031-7...ver20251114-8>

---

## ver20251031-7

### 更新内容

* feat: 新增调试和发布部署脚本
* style: assets 目录及其子目录的 Linguist 忽略设置
* docs: 添加仓库中不提供文件的说明
* fix: 更新版本号格式
* docs: 添加必要环境依赖说明及结构文件
* chore: 更新第三方库忽略规则
* style: 删除重复的注释行
* style: 添加第三方库忽略规则并移除旧路径
* docs: 移除 Live2DCubismCore 头文件引用
* feat(ui): 添加托盘报时功能
* fix: 删除不必要的 assets 复制步骤
* fix: 更改许可证标识为 GPLv3
* docs: 更新许可证链接格式
* fix: 更新项目许可证信息和版权声明

**完整更新日志：** <https://gitee.com/Pfolg/Pelr/compare/ver0.5-251018...ver20251031-7>

---

## ver0.5-251018

### 更新内容

* feat(ui): 添加启动项编辑标签与部署脚本
* fix(core/keyLabel): 移除静默模式 keyLabel 功能控制
* docs: 更新完整更新日志并调整版本号
* docs: 更新 README.md 中的安装说明
* docs: 添加完整版本更新日志
* feat(ui): 添加许可证检查功能

**完整更新日志：** <https://gitee.com/Pfolg/Pelr/compare/ver0.0.5...ver0.5-251018>

---

## ver0.0.5

**本次更新：** 完成项目适配 Gitee。UI 和项目结构优化。

已在 Windows 10 上经过测试，可正常运行。本次为非重大更新，无新功能的添加或移除。

### 更新内容

* refactor: 移除不必要的样式和代码调整
* style: 修正 README.md 的格式和注释内容
* refactor(ui): 将 QMenu 替换为 CustomMenu 并应用自定义样式
* feat(tray): 将启动菜单添加到托盘
* fix(ui): KeyLabel.cpp 单字符键名添加间距
* refactor: 移除冗余代码并优化 slider 访问方式
* refactor(ui): 窗口标志设置调整与 .idea 忽略规则更新
* docs: 更新文档中图片路径和编译配置说明
* feat(version): 添加对 Gitee 版本检查的支持
* docs: 更新构建流程中的默认设置
* feat(ui): 添加设置思考文本的功能
* refactor: 重构 launchByPathAsync 函数
* docs: 更新 README 链接
* fix(README): 更新下载安装链接以及目录结构
* docs: 更新打包指南并更新相关配置
* docs: 允许创建空白问题
* docs: 更新问题模板中的文档链接和版本选项
* docs: 添加 Bug 反馈和功能建议模板
* chore: 删除不必要的 Qt 动态链接库文件
* chore: 添加必要的组件
* docs: 更新社区准则和安全报告链接

**完整更新日志：** <https://gitee.com/Pfolg/Pelr/compare/ver0.0.4b...ver0.0.5>

---

## ver0.0.4b

由于 GitHub 账户被标记，项目迁移至 Gitee，原仓库可能无法访问。

**本次更新：**

* 添加媒体播放功能
* 原有的**关于**功能失效（将在后续更新修复）

之前的提交记录可尝试访问 [GitHub](https://github.com/Pfolg/Plauncher/commits/main/)（可能无效）。

---

## ver0.0.3

**本次更新：**

* 表情系统
* 检查更新功能
* 局部优化

**完整更新日志：** <https://github.com/Pfolg/Plauncher/compare/ver0.0.2...ver0.0.3>

---

## ver0.0.2

> **已知问题：** 部分额外窗口的大小不会根据设定自动调整，建议使用默认大小。

* TODO 功能实现
* UI 优化
* 托盘功能优化
* 第二聊天功能

**使用方法：** <https://github.com/Pfolg/Plauncher/wiki/Usage-Guide>

**完整更新日志：** <https://github.com/Pfolg/Plauncher/compare/ver0.0.1...ver0.0.2>

---

## ver0.0.1

> **已知问题：** 该版本的"随系统启动"功能无效，对基本使用无影响。

如提示缺失 `*.dll` 文件，可从 [bin_dlls](https://github.com/Pfolg/Plauncher/tree/main/bin_dlls) 目录下载对应文件并放置于程序根目录。同时请提交相关错误信息（界面 -> 设置 -> 关于 -> 问题反馈）。

首次运行时提示缺失模型属于正常现象，在设置中完成初次配置后即不再提示。

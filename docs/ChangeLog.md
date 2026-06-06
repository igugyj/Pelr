## v0.6.1 - 2026-06-06

### 新特性

- **多语言界面** — 界面已支持简体中文与英文动态切换
- **唇形模拟** — 播放语音时模型嘴部自动张合，模拟说话口型动态
- **语言模块正式可用** — 完整的多语言切换功能已集成

### 重构与优化

- **翻译模块重构** — 优化 Qt6 翻译配置，完善 CMake 构建流程
- **构建系统优化** — 新增 `.lupdate` 清理规则，完善构建产物清理脚本
- **文档更新** — 更新 Roadmap 标记已完成功能（快捷键、热加载配置）

### 问题修复

- **修复系统监控面板按钮尺寸异常** — 调整 `Clear` 和 `Clear All` 按钮自适应大小

### 文档与仓库

- **第三方依赖** — 新增 miniaudio 单文件音频解码库及相关鸣谢
- **开发者指引** — 完善贡献与开发文档

### 其他

- **引入新依赖** — 新增 miniaudio 子模块，需执行 `git submodule update --init --recursive`
- **版本迭代** — 自 v0.4.1 起累计 6 项功能增强与修复

## What's Changed

- release: v0.6.1 by @igugyj in <https://github.com/igugyj/Pelr/pull/29>

**Full Changelog**: <https://github.com/igugyj/Pelr/compare/v0.4.1...v0.6.1>

## v0.4.1

### 新特性

- **OpenAI-Compatible TTS** — 新增 OpenAI 兼容接口的 TTS 后端，支持自定义 endpoint、API key、模型和音色
- **系统监控面板** — 新增系统监控页面，实时显示内存占用率、磁盘占用率、进程内存详情及目录占用分析
- **随机 AI 问候** — 闲置时模型可主动发起 AI 问候对话（需配置 AI 服务）

### 重构与优化

- Qt 6 全面迁移适配
- 配置管理与部署流程重构
- 日志格式与风格统一优化
- 字体加载逻辑重构
- LLM 相关界面文案调整
- GitHub 用户名更新

### 问题修复

- 修复 voicevox TTS 缓存与模型加载问题
- 修复 Star 线程与 LookingMouseStrength 配置冲突
- 修复气泡文本格式化异常
- 修复点击坐标偏移
- 修复子进程未随父进程终止的问题

### 文档与仓库

- 更新 README（中英文）、SECURITY.md、SUPPORT.md、CONTRIBUTING.md
- 添加 Issue 与 PR 模板，完善贡献指南
- 移除不合规的许可证文件
- 添加脚本 `scripts/generate_notice.py`

### 其他

- 移除已分离的 Python TTS 服务端（移至独立仓库）
- 更新 `kissfft` 第三方库

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

- **文档更新：** 更新用户文档，提升使用指引的准确性与完整性。
- **TTS 设置页面：** 优化页面显示逻辑，改善界面交互体验。
- **VoiceVox 模型：** 优化模型加载时 StyleID 的载入逻辑，提高稳定性和正确性。
- **翻译功能：** 新增腾讯云翻译能力，并补充相关配置说明与文档支持。
- **翻译功能：** 基础支持 LibreTranslate。

**完整更新日志：** <https://github.com/igugyj/Pelr/compare/20260405.12.18...20260503.13.5>

---

## v20260405.12.18 - 2026-05-02

> **不兼容变更：** `configData.dat` 基本配置文件不兼容，需重新配置。

### 新特性

- **思考气泡动画：** 增加对话过程中的思考动画效果，交互更自然。
- **VoiceVox TTS 引擎支持：** 新增高质量语音合成后端，提供更多音色选择。
- **翻译后 TTS：** 可将输出文本先翻译为指定语言，再通过 TTS 朗读（Qt 界面配置，Python 端实现）。
- **自定义 AI 接口（OpenAI 兼容）：** 允许用户接入自定义 API 地址和密钥。
- **多源更新检查：** 支持从多个备用源检测新版本，提高更新可靠性。
- **通知提示音：** 新增系统通知时的可选提示音。

### 重构与优化

- **资源结构压缩：** 优化资源打包方式，显著减小构建产物体积。
- **全新 QSS 界面样式：** 引入 QSS 库重写样式表，视觉效果和一致性提升。
- **启动菜单布局优化：** 调整菜单间距与位置，减少误触概率。
- **托盘样式逻辑重构：** 规范托盘图标的激活、隐藏及消息处理逻辑。
- **设置页面 TTS 选项卡：** 重新排版 TTS 相关设置项，操作更清晰。
- **关于对话框：** 将 Markdown 文本浏览器设为自动打开外部链接。

### 问题修复

- **日志 UTF-8 支持：** 修复日志文件在 Windows 下编码乱码的问题。
- **默认气泡颜色：** 修正初始对话气泡的颜色显示异常。

### 文档与仓库

- 更新 `CREDITS.md` 与 `THANKS.md`，新增译者名单。
- 移除 Wiki，内容合并入主文档目录。
- 添加 Issue 与 PR 模板，规范社区贡献流程。
- 仓库地址从 Gitee 迁移至 GitHub，同步更新相关链接与说明。

### 依赖项变动

- 引入 `kissfft` 替代原有系统音频特征提取库，提升跨平台稳定性。

### 开发环境

- 开发工具由 CLion 替换为 VSCode，相关配置更新。

**完整更新日志：** <https://github.com/igugyj/Pelr/compare/20260219.11.17...20260405.12.18>

---

## v20260219.11.17 - 2026-04-01

> **不兼容变更：** `configData.dat` 基本配置文件不兼容，需重新配置。`ttsConfig.json` 虽兼容但建议重新配置以确保最佳体验。

### 新特性

- 音乐托盘支持颜色和符号自定义设置，优化气泡颜色逻辑。
- 新增 `openai_edge_tts` 语音合成引擎。
- 托盘菜单增加"打开用户文件夹"选项。
- TTS 连接失败时自动尝试启动服务端，并增加相关配置。

### 优化

- 控制界面适配响应式布局。
- 重构语句文件模块，提升气泡消息稳定性。
- 重构按键监听功能，优化按键提示体验。
- 窗口显示时尝试提升至最前。
- 启动线程弹窗标题及检查更新弹窗逻辑优化。

### 问题修复

- 修复启动时意外显示计划外时间气泡的问题。
- 修复按键标签中符号与键名间缺少空格的问题。
- 修正 README 中的错别字。

### 重构

- 项目重命名为 **Pelr**（原名 PLauncher）。
- 重构 src 目录结构。

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

- fix: 窗口图标不显示
- fix(core): 修改时间语录文件路径并添加新文件
- refactor: 删除不再使用的文本文件并更新语言设置
- refactor: 移除未使用函数并添加焦点检查功能
- style(core): 添加国际化支持并调整代码格式
- style: 修改文件头注释
- feat(translations): 修改配置文件
- fix(core): 修正窗口重置位置逻辑
- fix: 修改默认文本为 "Hello World!"
- fix(log): 日志信息国际化
- fix: 优化日志信息并添加多语言支持待办
- feat(todoNotification): TODO 消息使用托盘提醒（可选）
- style/docs (thirdParty): 移除图标链接的图片标签，优化 Markdown 显示
- docs: 修改 README 中的 NOTE 格式
- docs: 修改 README 中的术语和格式
- style(ui): 移除重复包含的 data.hpp
- feat(ui): 实现菜单自动隐藏功能
- feat(setting): 添加静默启动选项

**完整更新日志：** <https://gitee.com/Pfolg/Pelr/compare/ver20251114-8...ver20251118.9>

---

## ver20251114-8

### 更新内容

- release: ver20251114-8
- chore: 删除旧部署脚本并更新版本号及忽略规则
- refactor(启动项目菜单): 使用 CustomMenu 替换 QMenu
- docs: 添加 README_en.txt 和 README_sourceforge.txt
- docs: 更新发布日志

**完整更新日志：** <https://gitee.com/Pfolg/Pelr/compare/ver20251031-7...ver20251114-8>

---

## ver20251031-7

### 更新内容

- feat: 新增调试和发布部署脚本
- style: assets 目录及其子目录的 Linguist 忽略设置
- docs: 添加仓库中不提供文件的说明
- fix: 更新版本号格式
- docs: 添加必要环境依赖说明及结构文件
- chore: 更新第三方库忽略规则
- style: 删除重复的注释行
- style: 添加第三方库忽略规则并移除旧路径
- docs: 移除 Live2DCubismCore 头文件引用
- feat(ui): 添加托盘报时功能
- fix: 删除不必要的 assets 复制步骤
- fix: 更改许可证标识为 GPLv3
- docs: 更新许可证链接格式
- fix: 更新项目许可证信息和版权声明

**完整更新日志：** <https://gitee.com/Pfolg/Pelr/compare/ver0.5-251018...ver20251031-7>

---

## ver0.5-251018

### 更新内容

- feat(ui): 添加启动项编辑标签与部署脚本
- fix(core/keyLabel): 移除静默模式 keyLabel 功能控制
- docs: 更新完整更新日志并调整版本号
- docs: 更新 README.md 中的安装说明
- docs: 添加完整版本更新日志
- feat(ui): 添加许可证检查功能

**完整更新日志：** <https://gitee.com/Pfolg/Pelr/compare/ver0.0.5...ver0.5-251018>

---

## ver0.0.5

**本次更新：** 完成项目适配 Gitee。UI 和项目结构优化。

已在 Windows 10 上经过测试，可正常运行。本次为非重大更新，无新功能的添加或移除。

### 更新内容

- refactor: 移除不必要的样式和代码调整
- style: 修正 README.md 的格式和注释内容
- refactor(ui): 将 QMenu 替换为 CustomMenu 并应用自定义样式
- feat(tray): 将启动菜单添加到托盘
- fix(ui): KeyLabel.cpp 单字符键名添加间距
- refactor: 移除冗余代码并优化 slider 访问方式
- refactor(ui): 窗口标志设置调整与 .idea 忽略规则更新
- docs: 更新文档中图片路径和编译配置说明
- feat(version): 添加对 Gitee 版本检查的支持
- docs: 更新构建流程中的默认设置
- feat(ui): 添加设置思考文本的功能
- refactor: 重构 launchByPathAsync 函数
- docs: 更新 README 链接
- fix(README): 更新下载安装链接以及目录结构
- docs: 更新打包指南并更新相关配置
- docs: 允许创建空白问题
- docs: 更新问题模板中的文档链接和版本选项
- docs: 添加 Bug 反馈和功能建议模板
- chore: 删除不必要的 Qt 动态链接库文件
- chore: 添加必要的组件
- docs: 更新社区准则和安全报告链接

**完整更新日志：** <https://gitee.com/Pfolg/Pelr/compare/ver0.0.4b...ver0.0.5>

---

## ver0.0.4b

由于 GitHub 账户被标记，项目迁移至 Gitee，原仓库可能无法访问。

**本次更新：**

- 添加媒体播放功能
- 原有的**关于**功能失效（将在后续更新修复）

之前的提交记录可尝试访问 [GitHub](https://github.com/Pfolg/Plauncher/commits/main/)（可能无效）。

---

## ver0.0.3

**本次更新：**

- 表情系统
- 检查更新功能
- 局部优化

**完整更新日志：** <https://github.com/Pfolg/Plauncher/compare/ver0.0.2...ver0.0.3>

---

## ver0.0.2

> **已知问题：** 部分额外窗口的大小不会根据设定自动调整，建议使用默认大小。

- TODO 功能实现
- UI 优化
- 托盘功能优化
- 第二聊天功能

**使用方法：** <https://github.com/Pfolg/Plauncher/wiki/Usage-Guide>

**完整更新日志：** <https://github.com/Pfolg/Plauncher/compare/ver0.0.1...ver0.0.2>

---

## ver0.0.1

> **已知问题：** 该版本的"随系统启动"功能无效，对基本使用无影响。

如提示缺失 `*.dll` 文件，可从 [bin_dlls](https://github.com/Pfolg/Plauncher/tree/main/bin_dlls) 目录下载对应文件并放置于程序根目录。同时请提交相关错误信息（界面 -> 设置 -> 关于 -> 问题反馈）。

首次运行时提示缺失模型属于正常现象，在设置中完成初次配置后即不再提示。

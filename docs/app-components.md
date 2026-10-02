# 组件安装

**组件**指 Pelr 运行需要、但**不随程序一起分发**的第三方资源。缺少组件时程序仍可启动，但对应功能不可用（设置页会显示"组件缺失"）。

设置界面 → **Components** 选项卡可查看状态并就地安装，本文是完整安装教程。

## 组件一览

| 组件 | 用途 | 缺少时的表现 |
|---|---|---|
| **Live2D Cubism Core** | 桌宠模型渲染（**必需**） | 模型无法加载，桌面不显示桌宠 |
| **VOICEVOX CORE** | 日语本地 TTS（可选） | 无法使用 voicevox 语音合成，设置页 voicevox 选项卡置灰 |

> Live2D Cubism Core 使用专有许可证，**禁止随程序分发**，必须自行下载。这是它不在安装包内的原因，不是配置错误。

## 快速检查

设置 → **Components** 选项卡，卡片右侧的状态文字含义：

| 状态 | 含义 | 操作 |
|---|---|---|
| **Installed**（绿色） | 已就位，可正常使用 | 无 |
| **Archive ready — extract to install**（橙色） | 文件夹里有安装包，尚未解压 | 点 **Extract now** |
| **Layout not recognized**（红色） | 目录存在但结构不对 | 点 **Open folder** 检查目录结构 |
| **Not installed**（灰色） | 未安装 | 按下方教程操作 |

三个按钮：

- **Extract now** — 解压文件夹内的安装包（仅在有安装包时可用）
- **Open folder** — 在资源管理器中打开该组件的安装目录
- **Delete archive** — 删除文件夹内的安装包

---

## 安装 Live2D Cubism Core

### 方式一：ZIP 一键安装（推荐）

1. 前往 [Live2D 官网](https://www.live2d.com/en/sdk/download/native/) 下载 **Cubism SDK for Native**（本项目实测版本 `CubismSdkForNative-5-r.5`）
2. 设置 → **Components** → Live2D 卡片 → **Open folder**，打开 `Live2D` 文件夹
3. 把下载的 SDK 压缩包**直接放进该文件夹**
4. 回到 Components 页，点 **Extract now**

解压后程序会自动在内容里递归查找 `Live2DCubismCore.dll`（优先 `x86_64` 路径，其次取体积最大的），并把它提升到规范位置；多余的中间目录会被清理。

> 程序**启动时也会自动**检查 `Live2D` 文件夹内的安装包并解压，所以放进去重启一次同样有效。

### 方式二：手动放置

从 SDK 中取出 Core 运行时，摆成如下结构（与 exe 同级）：

```
Pelr.exe
└── Live2D/
    ├── Live2DCubismCore.dll
    └── LICENSE.md
```

`LICENSE.md` 可选（程序随包会复制一份）；`Live2DCubismCore.dll` 必需。

兼容历史布局：把 `Live2DCubismCore.dll` 直接放在 exe 同级（不在 `Live2D/` 里）也能识别，但**不推荐**。

---

## 安装 VOICEVOX CORE（可选，日语 TTS）

### 1. 下载运行时

前往 [voicevox_core 0.17.0 发布页面](https://github.com/VOICEVOX/voicevox_core/releases/tag/0.17.0)，下载 `download-windows-x64.exe` 并运行，下载完成后会生成一个 `voicevox_core` 文件夹。

### 2. 放置目录

设置 → **Components** → VOICEVOX 卡片 → **Open folder**，把整个 `voicevox_core` 文件夹放进打开的目录：

```
Pelr.exe
└── voicevox_core/
    ├── c_api/
    │   └── lib/voicevox_core.dll      ← 规范位置
    ├── onnxruntime/
    │   └── lib/voicevox_onnxruntime.dll
    ├── dict/                          ← 词典，必需
    └── models/                        ← 模型，约 1.6 GB，必需
```

这是**规范布局**，即官方包解压后的原样结构。也兼容平铺写法（两个 DLL 直接放在 `voicevox_core/` 下），两种布局可混用。

### 3. 词典与模型

`dict` 与 `models` 随官方包一起下载，**不会由安装程序自动投放**：

- `voicevox_core/dict` — 词典
- `voicevox_core/models` — 声学模型，约 **1.6 GB**。程序构建期刻意不复制这个目录，需要手动放好

缺一项 voicevox 就不可用，Components 页卡片的 `Missing:` 会逐项列出缺什么。

### 4. 语音模型（可选）

可前往 [voicevox_vvm](https://github.com/VOICEVOX/voicevox_vvm) 或[官方网站](https://voicevox.hiroshiba.jp/)预览并选择语音模型。

### 关于 `local_voicevox.dll`

VOICEVOX 的 C API 无法被主程序直接动态加载，因此实现为一个独立插件：

```
Pelr.exe
└── plugins/
    └── local_voicevox.dll
```

该文件**随程序发布**（属于程序本体，不是需要下载的组件）。若它缺失，设置页的 voicevox 选项卡会整体置灰 —— 这与 dict/models 缺失的表现不同，后者仍可打开选项卡，以便在设置页内补齐。

---

## 为什么我下载的版本里没有组件

**Release 构建刻意不投放可选组件**，这是设计行为：

- Live2D Cubism Core 受许可证约束不可分发
- voicevox 模型约 1.6 GB，不应默认打进安装包

所以首次运行 Release 版本时看到"组件缺失"是正常的，按本文手动安装即可。日常开发用的 Debug 构建则会自动把 `thirdParty/` 下已就位的组件复制到输出目录。

## 常见问题

| 症状 | 原因 | 解法 |
|---|---|---|
| 桌宠不显示 | 缺 `Live2DCubismCore.dll` | 按上文安装 Live2D Cubism Core |
| 卡片显示 `Layout not recognized` | 目录在，但里面没有可用的 DLL | 点 **Open folder**，确认结构符合上文示例 |
| voicevox 选项卡整个置灰 | 缺插件 `plugins/local_voicevox.dll` | 重新安装程序；该文件应随包提供 |
| 卡片显示 `Missing: models` | 模型未放置 | 把官方包的 `voicevox_core/models` 整个复制过去 |
| 解压后仍显示未安装 | 放错了文件夹 | 用 **Open folder** 确认打开的目录，再把包放进去 |
| 提示 DLL 存在但加载失败 | 缺 VC++ 运行库 | 安装 [Microsoft Visual C++ Redistributable](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist) |

> 若设置页未显示语音风格但配置正确，属于正常现象，测试功能仍可正常使用。每个语音模型有其相应的使用条款，请在使用前自行查阅。

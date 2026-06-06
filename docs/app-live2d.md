# 模型界面

模型界面是应用程序的主窗口，负责渲染 Live2D 模型并提供主要的交互功能。

该界面包含多个独立运行的子窗口：

- **GLCore 窗口** — 基于 OpenGL 渲染的模型窗口
- **右键菜单** — 提供多个操作选项的上下文菜单
- **消息气泡** — 显示通知消息，不具备交互功能
- **对话框** — 聊天与 AI 交互面板
- **按键提示窗口** — 实时显示键盘输入
- **媒体播放器** — 音频和视频播放

## GLCore 窗口

用户可通过此窗口与模型进行交互。模型会根据交互类型播放对应的动作。如模型不支持相关功能，交互将不会产生显著效果。

长按并拖动可移动窗口位置。

当消息气泡播放语音时，模型的嘴部会根据语音时长自动张合，模拟说话的口型动态。该功能与具体的语音合成服务无关，无需额外配置。

## 右键菜单

右键点击模型可显示上下文菜单。窗口失去焦点时菜单自动隐藏。

- **界面** — 打开主设置窗口
- **EMO** — 打开表情和动作选择菜单
- **启动** — 打开启动菜单，运行预定义项目
- **按键监听** — 在屏幕底部显示键盘输入信息
- **媒体播放** — 打开音视频播放窗口

<details>
<summary>预览</summary>

![alt text](assets/image-3.png)

</details>

- **说点什么** — 显示预定义文本的气泡消息，可在设置中自定义

<details>
<summary>预览</summary>

![alt text](assets/image-2.png)

</details>

- **问个问题** — 显示预定义的问题选项

<details>
<summary>预览</summary>

![alt text](assets/image-4.png)

</details>

配置启动项后，右键菜单将显示额外条目：

<details>
<summary>预览</summary>

![alt text](assets/image.png) ![alt text](assets/image-1.png)

</details>

## 消息气泡

消息气泡用于显示预设的通知消息，不具备交互功能。可显示定时提醒、TODO 提醒、自定义文本、AI 回复消息以及错误信息等。

<details>
<summary>预览</summary>

![alt text](assets/image-5.png)

</details>

## 对话框

双击模型窗口可显示对话框，再次双击可隐藏。

<details>
<summary>预览</summary>

![alt text](assets/image-6.png)

</details>

该对话框需要配置 AI 模型后方可使用。

## 按键提示窗口

该窗口基于 Windows API 实现，以字幕形式实时显示用户的键盘输入。目前仅支持开启和关闭功能，不支持自定义设置。

<details>
<summary>预览</summary>

![alt text](assets/image-7.png)

</details>

## 媒体播放器

该窗口支持音频和视频播放。所需依赖需单独下载。如未检测到依赖，应用程序将引导用户前往相关仓库下载。如无需该功能可忽略。

<details>
<summary>预览</summary>

![alt text](assets/image-8.png)

</details>

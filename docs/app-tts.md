# TTS（文本转语音）

TTS 服务用于将文本合成为语音输出。

## 配置

在设置界面的 TTS 选项卡中进行配置。支持两种后端服务：

- **OpenAI-Edge-TTS** — 免费使用
- **讯飞 TTS** — 付费使用，音质较高

<details>
<summary>预览</summary>

![alt text](assets/image-17.png)

</details>

## TTS 服务程序

TTS 服务程序是一个基于 Python 的本地网络服务器。该程序与讯飞和 Edge-TTS 供应商通信，接收合成的音频数据，并将本地文件路径返回给主程序进行播放。

该服务仅限本地使用，返回的是本地文件路径，不适用于跨设备的 TTS 服务。

<details>
<summary>预览</summary>

![alt text](assets/image-18.png)

</details>

## 支持的服务

1. 讯飞 TTS
2. OpenAI-Edge-TTS
3. 翻译功能

> 其他 TTS 或翻译服务可通过 Pelr C++ 前端独立运行。

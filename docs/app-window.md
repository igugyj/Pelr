# 主窗口

主窗口包含多个子页面。

## 设置

设置界面提供基本的配置选项，用户可根据个人偏好进行调整。如有未覆盖的需求，可提交 Issue 或自行实现相应功能。

<details>
<summary>预览</summary>

![alt text](assets/image-12.png)
![alt text](assets/image-13.png)

</details>

## 启动项配置

本功能可配置链接和应用程序路径。支持四种条目类型：`APP`、`Link`、`Scripts`、`Star`，不支持自定义类型。

`Star` 类别的条目将在应用程序启动约一分钟后自动执行。如系统已运行超过 20 分钟，则不会执行。

<details>
<summary>预览</summary>

![alt text](assets/image-11.png)

</details>

## TODO

TODO 窗口支持用户设置简单的任务事项。配置完成后，消息气泡可根据这些事项显示提醒。

<details>
<summary>预览</summary>

![alt text](assets/image-10.png)

</details>

## 系统监控

系统监控是主设置窗口的默认页面，提供实时的系统资源概览，包含两个环形指示器，分别显示内存和磁盘占用比例。

- **程序内存** — 显示当前进程的物理内存、峰值内存、私有内存和虚拟内存占用
- **磁盘空间** — 列出程序各目录的占用详情（Resources、voicevox_core、TTS 缓存、日志、用户数据），支持一键清除 TTS 缓存和日志文件

## AI 聊天

该窗口提供 AI 聊天界面。使用前需先配置 AI 模型。

<details>
<summary>预览</summary>

![alt text](assets/image-9.png)

</details>

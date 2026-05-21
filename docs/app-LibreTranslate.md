# LibreTranslate 离线翻译服务配置指南（Python）

## 参考链接

- <https://github.com/LibreTranslate/LibreTranslate>
- <https://github.com/argosopentech/argospm-index>
- <https://www.argosopentech.com/argospm/index/>

---

## 概述

本文档介绍如何在 Windows 系统上通过 Python 虚拟环境部署完全离线的翻译服务。该服务支持中文与日文之间的双向翻译，可通过浏览器或第三方应用程序调用。

---

## 第一步：安装 Python

1. 访问 Python 官方网站：<https://www.python.org/downloads/>
2. 下载 **Windows 版本**（建议选择 Python 3.10 或 3.11，避免使用 Python 3.13 以确保兼容性）
3. 运行安装程序，**务必勾选** `Add Python to PATH`
4. 按默认设置完成安装

---

## 第二步：创建虚拟环境

1. 新建一个目录用于存放服务，例如 `D:\libretranslate_py`
2. 打开 **命令提示符（CMD）**，进入该目录：

   ```cmd
   cd /d D:\libretranslate_py
   ```

3. 创建 Python 虚拟环境：

   ```cmd
   python -m venv venv
   ```

4. 激活虚拟环境：

   ```cmd
   venv\Scripts\activate
   ```

   命令行提示符前出现 `(venv)` 前缀即表示激活成功。

---

## 第三步：安装 LibreTranslate

在虚拟环境激活状态下执行：

```cmd
pip install libretranslate
```

> 如下载速度较慢，可使用国内镜像源：
>
> ```cmd
> pip install libretranslate -i https://pypi.tuna.tsinghua.edu.cn/simple
> ```

---

## 第四步：启动服务并下载模型

**首次启动时，LibreTranslate 会自动下载所需的语言模型（中文、英文、日文），无需手动干预。**

在虚拟环境激活状态下执行：

```cmd
libretranslate --load-only zh,en,ja
```

服务启动后，终端输出类似以下内容：

```
Updating language models
Downloading Chinese -> English ...
Downloading English -> Japanese ...
...
Listening at: http://0.0.0.0:5000
```

> **注意：** 首次下载模型需要访问 GitHub 等外部网站。如网络较慢或下载失败，请参考下文常见问题部分。

---

## 第五步：验证安装

打开浏览器，访问 `http://localhost:5000`，应显示 LibreTranslate 的 Web 界面。

在语言下拉菜单中选择 "Chinese (zh)" 和 "Japanese (ja)"，输入文本测试翻译功能。

---

## 第六步：与第三方应用程序集成

- 翻译 API 地址：`http://localhost:5000/translate`
- 调用方式：HTTP POST，JSON 格式，示例如下：

  ```json
  { "q": "你好", "source": "zh", "target": "ja" }
  ```

- 任何支持 HTTP 请求的应用程序均可使用此离线翻译服务。

---

## 常见问题

**问：端口 5000 已被占用怎么办？**
答：指定其他端口启动：

```cmd
libretranslate --port 5001 --load-only zh,en,ja
```

随后访问 `http://localhost:5001` 即可。

**问：模型下载失败或速度极慢怎么办？**
答：如因网络限制无法正常下载，可尝试以下方法：

1. **使用代理**：启动前设置环境变量：

   ```cmd
   set HTTP_PROXY=http://代理地址:端口
   set HTTPS_PROXY=http://代理地址:端口
   libretranslate --load-only zh,en,ja
   ```

2. **更换时段重试**：网络状况可能在非高峰时段有所改善。

**问：如何关闭服务？**
答：在命令提示符窗口中按 `Ctrl + C`，然后输入 `y` 确认。

**问：如何添加更多语言（如韩语）？**
答：在 `--load-only` 参数后添加对应语言代码，例如 `--load-only zh,en,ja,ko`，服务将在下次启动时自动下载韩语模型。

---

## 附录

首次启动后，Web 界面底部会显示已加载的语言模型。如仅加载了中文和日文，界面中的语言选项将相应显示。

<details>
<summary>预览</summary>

![alt text](assets/image-20.png)

</details>

<details>
<summary>预览</summary>

![alt text](assets/image-21.png)

</details>

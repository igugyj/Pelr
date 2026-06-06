#!/usr/bin/env python3
"""
生成 NOTICE 文件的脚本。
从内置的第三方库信息中读取每个库的许可证文件，
并将内容直接嵌入到最终生成的纯文本 NOTICE 文件中。
"""

import os
from datetime import datetime

# ============================================================
# 项目自身信息
# ============================================================
PROJECT_NAME = "Pelr"
PROJECT_AUTHOR = "SY Cheng"
PROJECT_HOMEPAGE = "https://github.com/igugyj/Pelr"

# ============================================================
# MIT 许可证全文 (用于项目自身声明)
# ============================================================
MIT_LICENSE_TEXT = """MIT License

Copyright (c) {year} {author}

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
""".format(year=datetime.now().year, author=PROJECT_AUTHOR)

# 内置标准许可证全文（用于 kissfft fallback 等）
BSD_3_CLAUSE_TEXT = """Copyright (c) <year> <copyright holders>

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software without
   specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
THE POSSIBILITY OF SUCH DAMAGE."""

ONNX_RUNTIME_MIT_TEXT = """MIT License

Copyright (c) Microsoft Corporation

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE."""

# ============================================================
# 第三方库信息列表
# ============================================================
THIRD_PARTY_LIBS = [
    {
        "name": "Qt",
        "version": "6.10.1",
        "license_type": "LGPLv3 (with GPLv2 as an alternative)",
        "copyright": "The Qt Company Ltd.",
        "homepage": "https://www.qt.io/",
        "license_path": "",
        "extra_notes": (
            "This software dynamically links to Qt libraries.\n"
            "You may obtain the source code of Qt and its full license texts from https://www.qt.io/."
        ),
    },
    {
        "name": "Live2D Cubism Native Framework",
        "version": "5-r.5",
        "license_type": "Live2D Open Software License",
        "copyright": "Live2D Inc.",
        "homepage": "https://www.live2d.com/",
        "license_path": "thirdParty/CubismNativeFramework/LICENSE.md",
        "extra_notes": (
            "If your annual revenue exceeds 10 million JPY, a separate Cubism SDK Release License is required.\n"
            "See https://www.live2d.com/en/download/cubism-sdk/release-license/"
        ),
    },
    {
        "name": "Live2D Cubism Core",
        "version": "Included with SDK",
        "license_type": "Live2D Proprietary Software License",
        "copyright": "Live2D Inc.",
        "homepage": "https://www.live2d.com/",
        "license_path": "",
        "extra_notes": (
            "Cubism Core is under a proprietary license. It must not be redistributed in public repositories.\n"
            "Obtain it from the official Live2D SDK. License text:\n"
            "https://www.live2d.com/eula/live2d-proprietary-software-license-agreement_en.html"
        ),
    },
    {
        "name": "GLEW",
        "version": "2.1.0",
        "license_type": "Modified BSD License",
        "copyright": "Milan Ikits, Marcelo E. Magallon, Lev Povalahev",
        "homepage": "http://glew.sourceforge.net/",
        "license_path": "thirdParty/glew/LICENSE.txt",
        "extra_notes": "",
    },
    {
        "name": "GLFW",
        "version": "3.4.0",
        "license_type": "zlib/libpng License",
        "copyright": "Marcus Geelnard, Camilla Löwy",
        "homepage": "https://www.glfw.org/",
        "license_path": "thirdParty/glfw/LICENSE.md",
        "extra_notes": "",
    },
    {
        "name": "kissfft",
        "version": "Latest",
        "license_type": "BSD-3-Clause",
        "copyright": "Mark Borgerding",
        "homepage": "https://github.com/mborgerding/kissfft",
        "license_path": "thirdParty/kissfft/COPYING",
        "extra_notes": "",
    },
    {
        "name": "miniaudio",
        "version": "0.11.25",
        "license_type": "Public Domain (www.unlicense.org) & MIT No Attribution",
        "copyright": "David Reid",
        "homepage": "https://github.com/mackron/miniaudio",
        "license_path": "thirdParty/miniaudio/LICENSE",
        "extra_notes": "",
    },
    {
        "name": "stb",
        "version": "Latest",
        "license_type": "MIT / Public Domain",
        "copyright": "Sean Barrett",
        "homepage": "https://github.com/nothings/stb",
        "license_path": "",
        "extra_notes": (
            "Each stb header file contains its own license notice at the top, typically public domain\n"
            "or the MIT License. Refer to the individual file for exact terms."
        ),
    },
    {
        "name": "VOICEVOX CORE",
        "version": "0.16.4",
        "license_type": "MIT (with additional terms)",
        "copyright": "Hiroshiba Kazuyuki",
        "homepage": "https://github.com/VOICEVOX/voicevox_core",
        "license_path": "thirdParty/voicevox_core/c_api/LICENSE",
        "extra_notes": (
            "YOU MUST INCLUDE THE CREDIT 'VOICEVOX: Hiroshiba Kazuyuki' when using the generated audio.\n"
            "Additional terms: see thirdParty/voicevox_core/c_api/TERMS.txt"
        ),
    },
    {
        "name": "ONNX Runtime",
        "version": "Included with voicevox_core",
        "license_type": "MIT (with VOICEVOX additional terms)",
        "copyright": "Microsoft Corporation",
        "homepage": "https://onnxruntime.ai/",
        # 注意：实际路径可能是 thirdParty/voicevox_core/onnxruntime/TERMS.txt
        # 如果不存在，脚本会提示但不会中断
        "license_path": "thirdParty/voicevox_core/onnxruntime/TERMS.txt",
        "extra_notes": "",
    },
    {
        "name": "Maple Mono (Maple Font)",
        "version": "V7",
        "license_type": "SIL Open Font License 1.1",
        "copyright": "The Maple Mono Project Authors",
        "homepage": "https://github.com/subframe7536/maple-font",
        "license_path": "public/font/OFL.txt",
        "extra_notes": "",
    },
    {
        "name": "IconaMoon",
        "version": "1.1",
        "license_type": "Free for any use",
        "copyright": "Dariush HPG",
        "homepage": "https://github.com/dariushhpg1/IconaMoon",
        "license_path": "",
        "extra_notes": "The README states: 'IconaMoon is totally free and you can do whatever you want with it. Take a copy and make it your own.'",
    },
]


# ============================================================
# 路径处理：从脚本位置向上查找项目根目录
# ============================================================
def find_project_root(start_path=None):
    """
    从 start_path 开始向上查找，直到找到一个包含 'thirdParty' 目录
    或 'README.md' 等特征文件的目录，作为项目根目录。
    如果找不到，返回 start_path 的父目录（若在 scripts 下）或 start_path 自身。
    """
    if start_path is None:
        start_path = os.path.dirname(os.path.abspath(__file__))
    current = os.path.abspath(start_path)
    # 最多向上找 10 层
    for _ in range(10):
        # 检查是否存在 thirdParty 目录（根目录标志）
        if os.path.isdir(os.path.join(current, "thirdParty")):
            return current
        # 或者检查是否存在典型的根目录文件
        if os.path.isfile(os.path.join(current, "README.md")) and os.path.isdir(
            os.path.join(current, "thirdParty")
        ):
            return current
        parent = os.path.dirname(current)
        if parent == current:  # 到达文件系统根
            break
        current = parent
    # 如果没找到，假设脚本在 scripts/ 下，则根目录为 scripts/..
    if os.path.basename(start_path).lower() in ("scripts", "script"):
        return os.path.dirname(start_path)
    return start_path


def read_license_file(relative_path, project_root):
    """读取相对于项目根目录的许可证文件内容，如果文件不存在则返回 None。"""
    if not relative_path:
        return None
    full_path = os.path.join(project_root, relative_path)
    if not os.path.exists(full_path):
        return None
    try:
        with open(full_path, 'r', encoding='utf-8', errors='replace') as f:
            return f.read()
    except Exception:
        return None


# ============================================================
# 主生成函数
# ============================================================
def generate_notice():
    """生成纯文本 NOTICE 文件内容。"""
    project_root = find_project_root()
    lines = []

    # 标题
    lines.append("NOTICE - Third Party Dependencies and Licenses")
    lines.append("=" * 60)
    lines.append(f"Project: {PROJECT_NAME}")
    lines.append(f"Homepage: {PROJECT_HOMEPAGE}")
    lines.append("")

    # 第一部分：项目自身的 MIT 许可证
    lines.append("1. Project License (MIT)")
    lines.append("-" * 40)
    lines.append(MIT_LICENSE_TEXT)
    lines.append("")

    # 第二部分：第三方库
    lines.append("2. Third Party Libraries")
    lines.append("-" * 40)

    for idx, lib in enumerate(THIRD_PARTY_LIBS, 1):
        name = lib["name"]
        version = lib["version"]
        license_type = lib["license_type"]
        copyright_holder = lib["copyright"]
        homepage = lib["homepage"]
        extra = lib.get("extra_notes", "")
        license_path = lib.get("license_path", "")

        lines.append(f"--- Library {idx}: {name} ---")
        if version:
            lines.append(f"Version: {version}")
        lines.append(f"License Type: {license_type}")
        lines.append(f"Copyright (c) {copyright_holder}")
        lines.append(f"Website: {homepage}")
        lines.append("")

        # 特殊处理：ONNX Runtime 需要独立输出 MIT + 附加条款
        if name == "ONNX Runtime":
            lines.append("---- ONNX Runtime MIT License ----")
            lines.append(ONNX_RUNTIME_MIT_TEXT.strip())
            lines.append("")
            terms_text = read_license_file(license_path, project_root)
            if terms_text:
                lines.append(
                    "---- Additional Terms Applicable to this Build (from VOICEVOX) ----"
                )
                lines.append(terms_text.strip())
                lines.append("")
            else:
                lines.append("---- Additional Terms (VOICEVOX) ----")
                lines.append(
                    "[NOT FOUND] thirdParty/voicevox_core/onnxruntime/TERMS.txt"
                )
                lines.append(
                    "Please refer to VOICEVOX CORE documentation for required credit and restrictions."
                )
                lines.append("")
            # 如果还有 extra_notes 则添加（此处为空，但保留逻辑）
            if extra:
                lines.append("Additional Notes:")
                lines.append(extra.strip())
                lines.append("")
            continue

        # 普通库：读取许可证文件
        license_text = read_license_file(license_path, project_root)
        if license_text:
            lines.append("---- Included License Text ----")
            lines.append(license_text.strip())
            lines.append("")
        else:
            # 对于 kissfft 提供 fallback
            if name == "kissfft" and license_path:
                lines.append("---- Included License Text (Fallback: BSD-3-Clause) ----")
                lines.append(BSD_3_CLAUSE_TEXT.strip())
                lines.append("")
            # 对于有 extra_notes 但无许可证文本的库（如 Qt, Live2D, stb），后面会输出 extra
        if extra:
            lines.append("Additional Notes:")
            lines.append(extra.strip())
            lines.append("")

    # 尾部说明
    lines.append("3. Additional Information")
    lines.append("-" * 40)
    lines.append(
        "Full license texts for the above libraries are maintained in their respective"
    )
    lines.append(
        "subdirectories under `thirdParty/`&`public/` in the source repository."
    )
    lines.append(
        "This NOTICE file is auto-generated by `generate_notice.py`. Please update the"
    )
    lines.append("script when dependencies change.")
    lines.append("")

    return "\n".join(lines)

# ============================================================
# 主程序入口
# ============================================================
if __name__ == "__main__":
    project_root = find_project_root()
    output_path = os.path.join(project_root, "NOTICE")
    content = generate_notice()
    with open(output_path, "w", encoding="utf-8") as f:
        f.write(content)
    print(f"NOTICE file generated successfully: {output_path}")

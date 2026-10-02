#!/usr/bin/env python3
"""
生成 NOTICE 文件的脚本。
- 库列表与项目信息：scripts/release_config.json（外部可维护数据）
- 许可证全文模板：licenses/ 目录（MIT.txt / BSD-3-Clause.txt / ONNX-Runtime-MIT.txt）
将每个库的许可证内容嵌入最终生成的纯文本 NOTICE 文件中。
"""

import os
from datetime import datetime

from release_config import load_config, find_project_root, resolve_path

# ============================================================
# 配置加载（无硬编码，全部来自 release_config.json）
# ============================================================

CONFIG = load_config()
PROJECT = CONFIG["project"]
PATHS = CONFIG["paths"]
THIRD_PARTY_LIBS = CONFIG["notice"]["third_party_libs"]
PROJECT_ROOT = find_project_root()


def license_template(fname):
    """读取 licenses/ 目录下的许可证全文模板；失败返回 None。"""
    path = resolve_path(PROJECT_ROOT, os.path.join(PATHS.get("licenses_dir", "licenses"), fname))
    try:
        with open(path, "r", encoding="utf-8") as f:
            return f.read()
    except (OSError, TypeError):
        return None


def read_license_file(relative_path):
    """读取相对于项目根目录的许可证文件内容，如果文件不存在则返回 None。"""
    if not relative_path:
        return None
    full_path = resolve_path(PROJECT_ROOT, relative_path)
    if not os.path.exists(full_path):
        return None
    try:
        with open(full_path, "r", encoding="utf-8", errors="replace") as f:
            return f.read()
    except Exception:
        return None


# ============================================================
# 主生成函数
# ============================================================
def generate_notice():
    """生成纯文本 NOTICE 文件内容。"""
    lines = []

    mit_text = license_template("MIT.txt")
    if mit_text:
        mit_text = mit_text.format(year=datetime.now().year, author=PROJECT["author"])
    else:
        mit_text = "[NOT FOUND] licenses/MIT.txt"

    bsd_fallback = license_template("BSD-3-Clause.txt") or "[NOT FOUND] licenses/BSD-3-Clause.txt"
    onnx_mit = license_template("ONNX-Runtime-MIT.txt") or "[NOT FOUND] licenses/ONNX-Runtime-MIT.txt"

    # 标题
    lines.append("NOTICE - Third Party Dependencies and Licenses")
    lines.append("=" * 60)
    lines.append(f"Project: {PROJECT['name']}")
    lines.append(f"Homepage: {PROJECT['homepage']}")
    lines.append("")

    # 第一部分：项目自身的 MIT 许可证
    lines.append("1. Project License (MIT)")
    lines.append("-" * 40)
    lines.append(mit_text.strip())
    lines.append("")

    # 第二部分：第三方库
    lines.append("2. Third Party Libraries")
    lines.append("-" * 40)

    for idx, lib in enumerate(THIRD_PARTY_LIBS, 1):
        name = lib["name"]
        version = lib.get("version", "")
        license_type = lib["license_type"]
        copyright_holder = lib["copyright"]
        homepage = lib.get("homepage", "")
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
            lines.append(onnx_mit.strip())
            lines.append("")
            terms_text = read_license_file(license_path)
            if terms_text:
                lines.append(
                    "---- Additional Terms Applicable to this Build (from VOICEVOX) ----"
                )
                lines.append(terms_text.strip())
                lines.append("")
            else:
                lines.append("---- Additional Terms (VOICEVOX) ----")
                lines.append(f"[NOT FOUND] {license_path}")
                lines.append(
                    "Please refer to VOICEVOX CORE documentation for required credit and restrictions."
                )
                lines.append("")
            if extra:
                lines.append("Additional Notes:")
                lines.append(extra.strip())
                lines.append("")
            continue

        # 普通库：读取许可证文件
        license_text = read_license_file(license_path)
        if license_text:
            lines.append("---- Included License Text ----")
            lines.append(license_text.strip())
            lines.append("")
        else:
            # 对于 kissfft 提供 fallback
            if name == "kissfft" and license_path:
                lines.append("---- Included License Text (Fallback: BSD-3-Clause) ----")
                lines.append(bsd_fallback.strip())
                lines.append("")
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
        "This NOTICE file is auto-generated by `generate_notice.py`. Please update"
    )
    lines.append(
        "`scripts/release_config.json` when dependencies change."
    )
    lines.append("")

    return "\n".join(lines)


# ============================================================
# 主程序入口
# ============================================================
def write_notice():
    """生成并写入 NOTICE 文件，返回输出路径。供脚本直接调用与 __main__ 共用。"""
    output_path = resolve_path(PROJECT_ROOT, PATHS.get("output", "NOTICE"))
    with open(output_path, "w", encoding="utf-8") as f:
        f.write(generate_notice())
    print(f"NOTICE file generated successfully: {output_path}")
    return output_path


if __name__ == "__main__":
    write_notice()

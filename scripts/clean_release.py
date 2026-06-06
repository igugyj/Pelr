#!/usr/bin/env python3
"""
Clean build artifacts from release/ directory.
Edit the CONFIG section below to customize rules.
"""

import os
import shutil
from pathlib import Path

# ============================================================
# CONFIG — 编辑此处自定义清理规则
# ============================================================

RELEASE_DIR = Path(__file__).resolve().parent.parent / "release"

# False = 仅预览（dry-run），True = 执行删除
RUN = True

# 要删除的目录（glob 模式）
DIRS = [
    "*_autogen",
    ".qt",
    ".cmake",
    "CMakeFiles",
    "log",
    "user",
    "voice_files",
    ".lupdate"
]

# 要删除的文件（glob 模式）
FILES = [
    "*.a",
    "cmake_install.cmake",
    "CMakeCache.txt",
    "compile_commands.json",
    "Makefile",
    "qrc_Resource.cpp",
    "Resource.qrc.depends",
    "*.qm",
    "*.h",
]

# 用户自定义规则（追加到此列表即可）
EXTRA_DIRS = []
EXTRA_FILES = []

# ============================================================
# 清理逻辑
# ============================================================


def fmt_size(size: int) -> str:
    if size < 1024:
        return f"{size} B"
    elif size < 1024 * 1024:
        return f"{size / 1024:.1f} KB"
    else:
        return f"{size / 1024 / 1024:.1f} MB"


def calc_dir_size(path: Path) -> int:
    total = 0
    for f in path.rglob("*"):
        if f.is_file():
            total += f.stat().st_size
    return total


def main():
    if not RELEASE_DIR.is_dir():
        print(f"[ERROR] release/ directory not found: {RELEASE_DIR}")
        return

    mode = "DRY-RUN" if not RUN else "DELETE"
    print(f"[{mode}] Cleaning: {RELEASE_DIR}")
    print()

    all_dirs = DIRS + EXTRA_DIRS
    all_files = FILES + EXTRA_FILES

    total_saved = 0
    total_items = 0

    # 清理目录
    for pattern in all_dirs:
        for entry in RELEASE_DIR.glob(pattern):
            if entry.is_dir():
                size = calc_dir_size(entry)
                total_saved += size
                total_items += 1
                print(f"  [dir]  {entry.name}  ({fmt_size(size)})")
                if RUN:
                    shutil.rmtree(entry, ignore_errors=True)

    # 清理文件
    for pattern in all_files:
        for entry in RELEASE_DIR.glob(pattern):
            if entry.is_file():
                size = entry.stat().st_size
                total_saved += size
                total_items += 1
                print(f"  [file] {entry.name}  ({fmt_size(size)})")
                if RUN:
                    entry.unlink(missing_ok=True)

    print()
    if total_items == 0:
        print("Nothing to clean.")
    else:
        verb = "Deleted" if RUN else "Would delete"
        print(f"{verb} {total_items} items, freed {fmt_size(total_saved)}.")


if __name__ == "__main__":
    main()

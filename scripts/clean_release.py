#!/usr/bin/env python3
"""
清理发布目录中的构建残留（从 scripts/release_config.json 读取规则）。
配置字段：paths.release_dir（目标目录）、clean.run / clean.dirs / clean.files。
clean.run 为 false 时仅预览（dry-run）。返回 True 表示清理流程成功执行（含预览）。
"""

import fnmatch
import os
import shutil

from release_config import load_config, find_project_root, resolve_path


def fmt_size(size: int) -> str:
    if size < 1024:
        return f"{size} B"
    elif size < 1024 * 1024:
        return f"{size / 1024:.1f} KB"
    else:
        return f"{size / 1024 / 1024:.1f} MB"


def calc_dir_size(path: str) -> int:
    total = 0
    for root, _dirs, files in os.walk(path):
        for f in files:
            full = os.path.join(root, f)
            total += os.path.getsize(full)
    return total


def clean_release() -> bool:
    """读取配置并清理发布目录；返回 True 表示流程执行成功（非目标目录缺失或异常）。"""
    config = load_config()
    release_dir = resolve_path(find_project_root(), config["paths"]["release_dir"])

    if not os.path.isdir(release_dir):
        print(f"[ERROR] release directory not found: {release_dir}")
        return False

    clean_cfg = config["clean"]
    run = bool(clean_cfg.get("run", True))
    dirs = clean_cfg.get("dirs", [])
    files = clean_cfg.get("files", [])

    mode = "DRY-RUN" if not run else "DELETE"
    print(f"[{mode}] Cleaning: {release_dir}")
    print()

    total_saved = 0
    total_items = 0

    # 清理目录
    for pattern in dirs:
        for entry in os.scandir(release_dir):
            if not entry.is_dir():
                continue
            if not fnmatch.fnmatch(entry.name, pattern):
                continue
            size = calc_dir_size(entry.path)
            total_saved += size
            total_items += 1
            print(f"  [dir]  {entry.name}  ({fmt_size(size)})")
            if run:
                shutil.rmtree(entry.path, ignore_errors=True)

    # 清理文件
    for pattern in files:
        for entry in os.scandir(release_dir):
            if not entry.is_file():
                continue
            if not fnmatch.fnmatch(entry.name, pattern):
                continue
            size = entry.stat().st_size
            total_saved += size
            total_items += 1
            print(f"  [file] {entry.name}  ({fmt_size(size)})")
            if run:
                os.remove(entry.path)

    print()
    if total_items == 0:
        print("Nothing to clean.")
    else:
        verb = "Deleted" if run else "Would delete"
        print(f"{verb} {total_items} items, freed {fmt_size(total_saved)}.")
    return True


if __name__ == "__main__":
    raise SystemExit(0 if clean_release() else 1)
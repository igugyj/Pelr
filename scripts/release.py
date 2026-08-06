#!/usr/bin/env python3
"""
release.py — 发布产物打包主流程（无参数，配置见 scripts/release_config.json）。

步骤：
  1) 生成 NOTICE（generate_notice.write_notice）
  2) 复制构建产物 build_dir -> release_dir（release.wipe_target_first 控制是否先清空目标）
  3) 清理发布目录残留（clean_release.clean_release，clean.run 控制是否真实删除）
"""

import os
import shutil

from release_config import load_config, find_project_root, resolve_path
from generate_notice import write_notice
from clean_release import clean_release


def copy_build(build_dir, release_dir, wipe_first):
    """将 build_dir 下的产物复制到 release_dir。"""
    if not os.path.isdir(build_dir):
        raise FileNotFoundError(f"[release] build directory not found: {build_dir}")

    if os.path.abspath(release_dir) == os.path.abspath(build_dir):
        raise ValueError("[release] build_dir and release_dir must differ")

    if os.path.exists(release_dir):
        if wipe_first:
            print(f"[release] Removing existing target: {release_dir}")
            shutil.rmtree(release_dir, ignore_errors=True)
        else:
            print(f"[release] Target exists, merging into: {release_dir}")
    else:
        os.makedirs(release_dir, exist_ok=True)

    shutil.copytree(build_dir, release_dir, dirs_exist_ok=True)
    print(f"[release] Copied {build_dir} -> {release_dir}")


def main():
    config = load_config()
    project_root = find_project_root()

    paths = config["paths"]
    build_dir = resolve_path(project_root, paths["build_dir"])
    release_dir = resolve_path(project_root, paths["release_dir"])
    release_cfg = config.get("release", {})
    wipe_first = bool(release_cfg.get("wipe_target_first", True))

    # 安全护栏：目标不得为项目根目录
    if os.path.abspath(release_dir) == os.path.abspath(project_root):
        raise ValueError("[release] refusal: release_dir cannot be the project root")

    # 步骤 1：生成 NOTICE
    write_notice()

    # 步骤 2：复制产物
    copy_build(build_dir, release_dir, wipe_first)

    # 步骤 3：清理残留
    if not clean_release():
        print("[release] warning: clean step failed (release directory may not exist)")

    print("[release] done.")


if __name__ == "__main__":
    main()
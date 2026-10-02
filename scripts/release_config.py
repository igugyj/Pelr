#!/usr/bin/env python3
"""
release_config.py — 共享配置加载器。

scripts/release.py / generate_notice.py / clean_release.py 统一从这里读取
外部可维护数据 scripts/release_config.json（路径相对本文件定位，不依赖 CWD）。
可通过环境变量 PELR_CONFIG 指定其他配置文件（用于测试/多分支）。
"""

import json
import os

_CONFIG_FILE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "release_config.json")


def config_path():
    return os.environ.get("PELR_CONFIG", _CONFIG_FILE)


def load_config():
    """加载 release_config.json，返回 dict。文件缺失或损坏时抛出明确错误。"""
    path = config_path()
    if not os.path.isfile(path):
        raise FileNotFoundError(f"[release_config] config file not found: {path}")
    try:
        with open(path, "r", encoding="utf-8") as f:
            return json.load(f)
    except json.JSONDecodeError as e:
        raise ValueError(f"[release_config] invalid JSON in {path}: {e}")


def find_project_root(start_path=None):
    """从 start_path 向上查找项目根目录（特征：存在 thirdParty 目录）。"""
    if start_path is None:
        start_path = os.path.dirname(os.path.abspath(__file__))
    current = os.path.abspath(start_path)
    for _ in range(10):
        if os.path.isdir(os.path.join(current, "thirdParty")):
            return current
        parent = os.path.dirname(current)
        if parent == current:  # 到达文件系统根
            break
        current = parent
    if os.path.basename(current).lower() in ("scripts", "script"):
        return os.path.dirname(current)
    return current


def resolve_path(project_root, rel_or_abs):
    """相对路径基于项目根目录解析，绝对路径原样返回。"""
    if not rel_or_abs:
        return project_root
    if os.path.isabs(rel_or_abs):
        return rel_or_abs
    return os.path.join(project_root, rel_or_abs)

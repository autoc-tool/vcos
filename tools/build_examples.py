# This Python file uses the following encoding: utf-8
import argparse
import json
import os
import subprocess
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
BUILD_DIR = os.path.join(ROOT, "build")
CONFIG_PATH = os.path.join(BUILD_DIR, "build_config.json")
VCOS_BUILD = os.path.join(BUILD_DIR, "vcos_build.py")


def load_config():
    with open(CONFIG_PATH, "r", encoding="utf-8") as f:
        return json.load(f)


def select_pairs(apps, app_names, board_names):
    selected = []
    for item in apps:
        if app_names and item["app_name"] not in app_names:
            continue
        if board_names and item["board_name"] not in board_names:
            continue
        selected.append(item)
    return selected


def build_one(app_name, board_name, compiler):
    cmd = [
        sys.executable,
        "-u",
        VCOS_BUILD,
        "-app_name",
        app_name,
        "-board_name",
        board_name,
        "-compiler",
        compiler,
        "-all",
    ]
    env = os.environ.copy()
    env.setdefault("VIRTUAL_ENV", sys.prefix)
    print(f"=== {app_name} / {board_name} ===", flush=True)
    return subprocess.call(cmd, env=env)


def main():
    parser = argparse.ArgumentParser(description="Build every example/board pair in build/build_config.json")
    parser.add_argument("--app", action="append", default=[], help="limit to this app name, repeatable")
    parser.add_argument("--board", action="append", default=[], help="limit to this board name, repeatable")
    parser.add_argument("--compiler", default=None, help="compiler name, default is build_config.json")
    args = parser.parse_args()

    cfg = load_config()
    compiler = args.compiler or cfg.get("compiler") or "gcc"
    selected = select_pairs(cfg["apps"], args.app, args.board)
    if not selected:
        print("no example/board pairs selected")
        return 1

    failed = []
    for item in selected:
        app_name = item["app_name"]
        board_name = item["board_name"]
        if build_one(app_name, board_name, compiler) != 0:
            failed.append(f"{app_name}:{board_name}")

    if failed:
        print("failed: " + ", ".join(failed))
        return 1
    print(f"built {len(selected)} example/board pair(s)")
    return 0


if __name__ == "__main__":
    sys.exit(main())

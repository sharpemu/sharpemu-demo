#!/usr/bin/env python3
import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SOURCES = ["src/main.c", "src/game.c"]
STATIC_FILES = ["param.json", "icon0.png", "pic0.png"]
FLAGS = [
    "-O2",
    "-ffreestanding",
    "-fno-math-errno",
    "-Wall",
    "-Wextra",
    "-Werror",
    "-Wno-unused-command-line-argument",
    "-nostartfiles",
    "-nodefaultlibs",
]
LIBRARIES = ["-lkernel", "-lSceVideoOut", "-lSceSystemService", "-lSceUserService", "-lScePad"]


def compiler(sdk: Path) -> Path:
    if os.name == "nt":
        return sdk / "win" / "prospero-clang.cmd"
    return sdk / "bin" / "prospero-clang"


def main() -> int:
    parser = argparse.ArgumentParser(description="Build the SharpEmu demo as an app folder with eboot.bin and sce_sys/.")
    parser.add_argument("output", type=Path, help="folder that receives eboot.bin and sce_sys/, outside the repository")
    parser.add_argument("--sdk", type=Path, default=os.environ.get("PS5_PAYLOAD_SDK"),
                        help="ps5-payload-sdk folder (default: $PS5_PAYLOAD_SDK)")
    args = parser.parse_args()

    if args.sdk is None:
        parser.error("set PS5_PAYLOAD_SDK or pass --sdk")
    clang = compiler(args.sdk.resolve())
    if not clang.exists():
        parser.error(f"{clang} does not exist")

    output = args.output.resolve()
    if output == ROOT or ROOT in output.parents:
        parser.error("the output folder must be outside the repository")

    sce_sys = output / "sce_sys"
    shutil.rmtree(sce_sys, ignore_errors=True)
    sce_sys.mkdir(parents=True)

    command = [str(clang), *FLAGS, "-o", str(output / "eboot.bin"), *(str(ROOT / source) for source in SOURCES), *LIBRARIES]
    result = subprocess.run(command)
    if result.returncode != 0:
        return result.returncode

    for name in STATIC_FILES:
        shutil.copy2(ROOT / "static" / name, sce_sys / name)

    print(f"{output / 'eboot.bin'}")
    print(f"{sce_sys}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

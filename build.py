#!/usr/bin/env python3
import argparse
import os
import shutil
import subprocess
import sys
import tempfile
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
IMPORTS = {
    "libkernel": {
        "sceKernelAllocateMainDirectMemory": "B+vc2AO2Zrc",
        "sceKernelMapDirectMemory": "L-Q3LEjIbgA",
        "sceKernelUsleep": "1jfXLRVzisc",
        "sceKernelGetProcessTime": "4J2sUJmuHZQ",
        "sceKernelCreateEqueue": "D0OdFMjp46I",
        "sceKernelWaitEqueue": "fzyMKs9kim0",
    },
    "libSceVideoOut": {
        "sceVideoOutOpen": "Up36PTk687E",
        "sceVideoOutSetFlipRate": "CBiu4mCE1DA",
        "sceVideoOutAddFlipEvent": "HXzjK9yI30k",
        "sceVideoOutSubmitFlip": "U46NwOiJpys",
        "sceVideoOutSetBufferAttribute2": "PjS5uASwcV8",
        "sceVideoOutRegisterBuffers2": "rKBUtgRrtbk",
    },
    "libSceSystemService": {"sceSystemServiceHideSplashScreen": "Vo5V8KAwCmk"},
    "libSceUserService": {
        "sceUserServiceInitialize": "j3YMu1MVNNo",
        "sceUserServiceGetInitialUser": "CdWp0oHWGr0",
    },
    "libScePad": {
        "scePadInit": "hv1luiJrqQM",
        "scePadOpen": "xk0AcarP3V4",
        "scePadReadState": "YndgXqQVV7c",
        "scePadSetLightBar": "RR4novUEENY",
    },
}


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

    with tempfile.TemporaryDirectory(prefix="sharpemu-demo-") as temporary:
        directory = Path(temporary)
        aliases = directory / "imports.h"
        aliases.write_text("\n".join(
            f'__asm__(".set {name}, \\"{nid}\\"");'
            for symbols in IMPORTS.values() for name, nid in symbols.items()
        ), encoding="ascii")
        libraries = []
        for module, symbols in IMPORTS.items():
            assembly = directory / f"{module}.S"
            assembly.write_text(".text\n" + "\n".join(
                f'.global "{nid}"\n.type "{nid}", @function\n"{nid}":\n    ret'
                for nid in symbols.values()
            ) + '\n.section .note.GNU-stack,"",@progbits\n', encoding="ascii")
            library = directory / f"{module}.so"
            # The Windows SDK linker forces PIE, which is incompatible with shared stubs.
            result = subprocess.run([
                str(clang), *FLAGS, "--target=x86_64-unknown-freebsd", "-fuse-ld=lld", "-shared",
                f"-Wl,-soname,{module}.sprx", str(assembly), "-o", str(library),
            ])
            if result.returncode != 0:
                return result.returncode
            libraries.append(str(library))

        command = [str(clang), *FLAGS, "-include", str(aliases), "-o", str(output / "eboot.bin"),
                   *(str(ROOT / source) for source in SOURCES), *libraries]
        result = subprocess.run(command)
        if result.returncode != 0:
            return result.returncode

    with (output / "eboot.bin").open("r+b") as elf:
        header = elf.read(64)
        if (len(header) != 64 or header[:7] != b"\x7fELF\x02\x01\x01"
                or int.from_bytes(header[18:20], "little") != 62):
            print("Compiler output is not a little-endian x86-64 ELF64 executable.", file=sys.stderr)
            return 1
        elf.seek(7)
        elf.write(b"\x09\x02")

    for name in STATIC_FILES:
        shutil.copy2(ROOT / "static" / name, sce_sys / name)

    print(f"{output / 'eboot.bin'}")
    print(f"{sce_sys}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

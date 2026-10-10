import contextlib
import io
import subprocess
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

import build


class BuildTests(unittest.TestCase):
    def test_ps5_header_and_invalid_compiler_outputs(self):
        valid = b"\x7fELF\x02\x01\x01\x00\x00" + bytes(9) + b"\x3e\x00" + bytes(44)
        fixtures = [
            (valid, 0),
            (valid[:63], 1),
            (b"SELF" + valid[4:], 1),
            (valid[:4] + b"\x01" + valid[5:], 1),
            (valid[:5] + b"\x02" + valid[6:], 1),
            (valid[:18] + b"\xb7\x00" + valid[20:], 1),
        ]
        for original, expected_result in fixtures:
            with self.subTest(header=original), tempfile.TemporaryDirectory() as temporary:
                root = Path(temporary)
                sdk = root / "sdk"
                clang = build.compiler(sdk)
                clang.parent.mkdir(parents=True)
                clang.touch()
                output = root / "app"

                def compile_demo(command):
                    if "-shared" in command:
                        assembly = next(Path(argument) for argument in command if argument.endswith(".S"))
                        symbols = build.IMPORTS[assembly.stem]
                        text = assembly.read_text(encoding="ascii")
                        for nid in symbols.values():
                            self.assertIn(f'.global "{nid}"', text)
                        self.assertIn(f"-Wl,-soname,{assembly.stem}.sprx", command)
                    else:
                        aliases = Path(command[command.index("-include") + 1]).read_text(encoding="ascii")
                        for symbols in build.IMPORTS.values():
                            for name, nid in symbols.items():
                                self.assertIn(f'.set {name}, \\"{nid}\\"', aliases)
                        self.assertEqual(sum(argument.endswith(".so") for argument in command), len(build.IMPORTS))
                    Path(command[command.index("-o") + 1]).write_bytes(original)
                    return subprocess.CompletedProcess(command, 0)

                with patch("sys.argv", ["build.py", str(output), "--sdk", str(sdk)]), \
                        patch("build.subprocess.run", side_effect=compile_demo), \
                        contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
                    self.assertEqual(build.main(), expected_result)

                expected = original[:7] + b"\x09\x02" + original[9:] if expected_result == 0 else original
                self.assertEqual((output / "eboot.bin").read_bytes(), expected)
                for name in build.STATIC_FILES:
                    self.assertEqual((output / "sce_sys" / name).exists(), expected_result == 0)
                    if expected_result == 0:
                        self.assertEqual((output / "sce_sys" / name).read_bytes(), (build.ROOT / "static" / name).read_bytes())
                if expected_result == 0:
                    self.assertEqual({file.name for file in (output / "sce_sys").iterdir()},
                                     {"param.json", "icon0.png", "pic0.png"})

    def test_link_failure_is_returned(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            sdk = root / "sdk"
            clang = build.compiler(sdk)
            clang.parent.mkdir(parents=True)
            clang.touch()
            output = root / "app"
            with patch("sys.argv", ["build.py", str(output), "--sdk", str(sdk)]), \
                    patch("build.subprocess.run", side_effect=[subprocess.CompletedProcess([], 7)]) as run:
                self.assertEqual(build.main(), 7)
                self.assertEqual(run.call_count, 1)
                self.assertFalse((output / "eboot.bin").exists())


if __name__ == "__main__":
    unittest.main()

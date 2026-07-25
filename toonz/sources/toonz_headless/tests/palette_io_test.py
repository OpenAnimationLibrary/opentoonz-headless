#!/usr/bin/env python3
"""Smoke test for Palette.save() and Palette.load()."""

import json
import os
import subprocess
import sys
from pathlib import Path

BINARY = Path(__file__).resolve().parents[2] / "build/bin/toonz_headless"
OUTPUT = Path("/tmp/opentoonz_headless_palette_io.tpl")

JS_CODE = r'''
var pal = new Palette();
pal.addPage("Landscape");
var red = pal.addColor(220, 40, 40, 255);
var green = pal.addColor(60, 160, 80, 255);
pal.save("/tmp/opentoonz_headless_palette_io.tpl");
print("saved styles=" + pal.styleCount);

var loaded = new Palette();
loaded.load("/tmp/opentoonz_headless_palette_io.tpl");
var c1 = loaded.getStyleColor(red);
var c2 = loaded.getStyleColor(green);
print("loaded styles=" + loaded.styleCount);
print("red=" + c1.r + "," + c1.g + "," + c1.b + "," + c1.a);
print("green=" + c2.r + "," + c2.g + "," + c2.b + "," + c2.a);
'''


def main() -> int:
    if not BINARY.is_file():
        print(f"ERROR: binary not found: {BINARY}", file=sys.stderr)
        return 2

    if OUTPUT.exists():
        OUTPUT.unlink()

    env = os.environ.copy()
    env.setdefault("QT_QPA_PLATFORM", "offscreen")

    requests = [
        {"jsonrpc": "2.0", "id": 1, "method": "eval", "params": {"code": JS_CODE}},
        {"jsonrpc": "2.0", "id": 2, "method": "quit", "params": {}},
    ]

    process = subprocess.Popen(
        [str(BINARY)],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        env=env,
    )

    payload = "".join(json.dumps(request) + "\n" for request in requests)
    stdout, stderr = process.communicate(payload)

    print(stdout, end="")
    if stderr:
        print(stderr, file=sys.stderr, end="")

    expected = [
        "saved styles=4",
        "loaded styles=4",
        "red=220,40,40,255",
        "green=60,160,80,255",
    ]

    success = process.returncode == 0 and OUTPUT.is_file()
    success = success and all(item in stdout for item in expected)

    if not success:
        print("Palette I/O smoke test FAILED", file=sys.stderr)
        return 1

    print(f"Palette I/O smoke test passed: {OUTPUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

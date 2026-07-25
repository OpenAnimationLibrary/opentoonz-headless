#!/usr/bin/env python3
"""Portable JSON-RPC smoke test for the OpenToonz headless executable."""

from __future__ import annotations

import json
import os
from pathlib import Path
import subprocess
import sys


def read_json_line(process: subprocess.Popen[str]) -> dict:
    assert process.stdout is not None
    line = process.stdout.readline()
    if not line:
        stderr = process.stderr.read() if process.stderr else ""
        raise RuntimeError(
            f"Headless process ended before returning JSON.\n{stderr}"
        )
    return json.loads(line)


def main() -> int:
    if len(sys.argv) != 2:
        print(f"Usage: {Path(sys.argv[0]).name} <toonz_headless executable>", file=sys.stderr)
        return 2

    binary = Path(sys.argv[1]).resolve()
    if not binary.is_file():
        print(f"Executable not found: {binary}", file=sys.stderr)
        return 2

    env = os.environ.copy()
    env["QT_QPA_PLATFORM"] = "offscreen"

    process = subprocess.Popen(
        [str(binary)],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        bufsize=1,
        env=env,
    )

    try:
        ready = read_json_line(process)
        assert ready.get("method") == "ready", ready

        requests = [
            {"id": 1, "method": "ping", "params": {}},
            {"id": 2, "method": "eval", "params": {"code": "print(6 * 7)"}},
            {"id": 3, "method": "quit", "params": {}},
        ]

        responses: dict[int, dict] = {}
        assert process.stdin is not None
        for request in requests:
            process.stdin.write(json.dumps(request) + "\n")
            process.stdin.flush()
            response = read_json_line(process)
            responses[int(response["id"])] = response

        assert responses[1]["result"] == "pong", responses[1]
        assert responses[2]["result"]["ok"] is True, responses[2]
        assert responses[2]["result"]["output"].strip() == "42", responses[2]
        assert responses[3]["result"] == "bye", responses[3]

        return_code = process.wait(timeout=15)
        assert return_code == 0, return_code
        print(f"JSON-RPC smoke test passed: {binary}")
        return 0
    except Exception:
        process.kill()
        stderr = process.stderr.read() if process.stderr else ""
        if stderr:
            print(stderr, file=sys.stderr)
        raise


if __name__ == "__main__":
    raise SystemExit(main())

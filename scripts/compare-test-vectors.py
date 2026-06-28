#!/usr/bin/env python3
"""Compare ZK-ARCHE JSON test-vector corpora across implementations."""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path


REQUIRED_TOP_LEVEL_KEYS = {"suite", "name", "inputs", "expected"}


def load_json(path: Path) -> object:
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        raise ValueError(f"{path}: invalid JSON: {exc}") from exc


def json_files(path: Path) -> dict[str, Path]:
    if not path.is_dir():
        raise ValueError(f"{path}: not a directory")
    return {p.name: p for p in sorted(path.glob("*.json"))}


def canonical(value: object) -> str:
    return json.dumps(value, sort_keys=True, separators=(",", ":"), ensure_ascii=False)


def validate_vector(path: Path, value: object) -> None:
    if not isinstance(value, dict):
        raise ValueError(f"{path}: vector must be a JSON object")
    missing = REQUIRED_TOP_LEVEL_KEYS - value.keys()
    if missing:
        raise ValueError(f"{path}: missing top-level keys: {', '.join(sorted(missing))}")
    if value["suite"] != "0x0001":
        raise ValueError(f"{path}: unsupported suite {value['suite']!r}")


def compare(left: Path, right: Path) -> int:
    left_files = json_files(left)
    right_files = json_files(right)

    failures: list[str] = []
    left_names = set(left_files)
    right_names = set(right_files)
    for name in sorted(left_names - right_names):
        failures.append(f"{right}: missing {name}")
    for name in sorted(right_names - left_names):
        failures.append(f"{left}: missing {name}")

    for name in sorted(left_names & right_names):
        try:
            left_value = load_json(left_files[name])
            right_value = load_json(right_files[name])
            validate_vector(left_files[name], left_value)
            validate_vector(right_files[name], right_value)
        except ValueError as exc:
            failures.append(str(exc))
            continue

        if canonical(left_value) != canonical(right_value):
            failures.append(f"{name}: vector contents differ")

    if failures:
        print("test-vector comparison failed:", file=sys.stderr)
        for failure in failures:
            print(f"  - {failure}", file=sys.stderr)
        return 1

    print(
        f"test-vector corpora match: {left} == {right} "
        f"({len(left_files)} JSON vectors)"
    )
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--left",
        type=Path,
        default=Path("rust/test-vectors/0x0001"),
        help="left vector corpus",
    )
    parser.add_argument(
        "--right",
        type=Path,
        default=Path("python/test-vectors/0x0001"),
        help="right vector corpus",
    )
    args = parser.parse_args()
    return compare(args.left, args.right)


if __name__ == "__main__":
    raise SystemExit(main())

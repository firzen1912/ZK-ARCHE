#!/usr/bin/env python3
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
FUZZ = ROOT / "rust" / "fuzz"
CARGO = FUZZ / "Cargo.toml"
TARGETS = FUZZ / "fuzz_targets"
CORPUS = FUZZ / "corpus"


def fail(msg: str) -> None:
    print(f"FAIL fuzz provenance: {msg}", file=sys.stderr)
    raise SystemExit(1)


text = CARGO.read_text(encoding="utf-8")
blocks = re.split(r"(?=^\[\[bin\]\]\s*$)", text, flags=re.MULTILINE)
registered = {}
for block in blocks:
    if not block.startswith("[[bin]]"):
        continue
    name_m = re.search(r'^\s*name\s*=\s*"([^"]+)"\s*$', block, re.MULTILINE)
    path_m = re.search(r'^\s*path\s*=\s*"([^"]+)"\s*$', block, re.MULTILINE)
    if not name_m or not path_m:
        fail("every [[bin]] fuzz target must declare name and path")
    name = name_m.group(1)
    path = path_m.group(1)
    if name in registered:
        fail(f"duplicate fuzz target registration: {name}")
    registered[name] = path

if not registered:
    fail("no fuzz targets registered")

for name, relpath in sorted(registered.items()):
    src = FUZZ / relpath
    if not src.is_file():
        fail(f"registered target {name} missing source {relpath}")
    expected = Path("fuzz_targets") / f"{name}.rs"
    if Path(relpath) != expected:
        fail(f"target {name} path must be {expected}, got {relpath}")

    source_text = src.read_text(encoding="utf-8")
    if not re.search(r"(?m)^\s*#!\[no_main\]\s*$", source_text):
        fail(f"target {name} is not a no_main libFuzzer harness")
    if not re.search(
        r"(?m)^\s*use\s+libfuzzer_sys::fuzz_target\s*;\s*$", source_text
    ):
        fail(f"target {name} does not import libfuzzer_sys::fuzz_target")
    fuzz_entries = re.findall(r"\bfuzz_target!\s*\(", source_text)
    if len(fuzz_entries) != 1:
        fail(
            f"target {name} must contain exactly one fuzz_target! entry point, "
            f"found {len(fuzz_entries)}"
        )
    if "proto::" not in source_text:
        fail(f"target {name} does not reference production proto code")

    corpus_dir = CORPUS / name
    if not corpus_dir.is_dir():
        fail(f"target {name} missing corpus namespace")
    seeds = [p for p in corpus_dir.iterdir() if p.is_file() and p.stat().st_size > 0]
    if not seeds:
        fail(f"target {name} has no non-empty retained seed")

target_sources = {p.stem for p in TARGETS.glob("*.rs") if p.is_file()}
if target_sources != set(registered):
    fail(
        "fuzz target source/registration drift: "
        f"registered={sorted(registered)} sources={sorted(target_sources)}"
    )

corpus_names = {p.name for p in CORPUS.iterdir() if p.is_dir()}
if corpus_names != set(registered):
    fail(
        "fuzz corpus/registration drift: "
        f"registered={sorted(registered)} corpora={sorted(corpus_names)}"
    )

print(
    "PASS fuzz provenance: "
    f"targets={len(registered)} "
    f"names={','.join(sorted(registered))}"
)

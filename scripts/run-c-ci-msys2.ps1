param(
    [string]$MsysRoot = "C:\tmp\msys64"
)

$ErrorActionPreference = "Stop"

$bash = Join-Path $MsysRoot "usr\bin\bash.exe"
if (-not (Test-Path -LiteralPath $bash)) {
    throw "MSYS2 bash was not found at $bash. Run scripts\setup-msys2-c-env.ps1 first."
}

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$repoRootForBash = (& $bash -lc "cygpath -u '$($repoRoot.Path)'").Trim()

& $bash -lc "set -euo pipefail; export PATH=/ucrt64/bin:/usr/bin:`$PATH; cd '$repoRootForBash'; for t in make pkg-config gcc clang cppcheck; do command -v `$t >/dev/null; done; bash ./scripts/ci-c.sh"

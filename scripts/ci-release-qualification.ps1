param(
    [string]$Bash = ""
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot

if (-not $Bash) {
    $candidates = @(
        "C:\tmp\msys64\usr\bin\bash.exe",
        "C:\Program Files\Git\bin\bash.exe",
        "C:\Program Files\Git\usr\bin\bash.exe",
        "bash"
    )
    foreach ($candidate in $candidates) {
        if ($candidate -eq "bash") {
            $cmd = Get-Command bash -ErrorAction SilentlyContinue
            if ($cmd) {
                $Bash = $cmd.Source
                break
            }
        } elseif (Test-Path -LiteralPath $candidate) {
            $Bash = $candidate
            break
        }
    }
}

if (-not $Bash) {
    throw "Git Bash or another bash executable is required to run release qualification locally."
}

function Convert-ToBashPath([string]$Path) {
    $resolved = (Resolve-Path -LiteralPath $Path).Path
    $drive = $resolved.Substring(0, 1).ToLowerInvariant()
    $tail = $resolved.Substring(2).Replace("\", "/")
    return "/$drive$tail"
}

if (-not $env:PYTHON) {
    $pythonCandidates = @(
        "$root\.venv\Scripts\python.exe",
        "$env:LOCALAPPDATA\HermesLegionCommander\venv\Scripts\python.exe",
        "$env:LOCALAPPDATA\hermes\hermes-agent\venv\Scripts\python.exe",
        "$env:LOCALAPPDATA\LegionCommander\venv\Scripts\python.exe"
    )
    foreach ($candidate in $pythonCandidates) {
        if (Test-Path -LiteralPath $candidate) {
            & $candidate -V *> $null
            if ($LASTEXITCODE -eq 0) {
                $env:PYTHON = Convert-ToBashPath $candidate
                break
            }
        }
    }
}

if (Test-Path -LiteralPath "C:\tmp\msys64\ucrt64\bin") {
    $env:PATH = "C:\tmp\msys64\ucrt64\bin;C:\tmp\msys64\usr\bin;$env:PATH"
}
if (Test-Path -LiteralPath "C:\Program Files\Git\cmd") {
    $env:PATH = "C:\Program Files\Git\cmd;$env:PATH"
}
if (Test-Path -LiteralPath "$env:USERPROFILE\.cargo\bin") {
    $env:PATH = "$env:USERPROFILE\.cargo\bin;$env:PATH"
}
if (-not $env:GIT -and (Test-Path -LiteralPath "C:\Program Files\Git\cmd\git.exe")) {
    $env:GIT = Convert-ToBashPath "C:\Program Files\Git\cmd\git.exe"
}

& $Bash "$root/scripts/ci-release-qualification.sh"
exit $LASTEXITCODE

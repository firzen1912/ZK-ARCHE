param(
    [string]$MsysRoot = "C:\tmp\msys64",
    [string]$DownloadDir = "C:\tmp"
)

$ErrorActionPreference = "Stop"

$installerName = "msys2-base-x86_64-20260611.sfx.exe"
$installerUrl = "https://github.com/msys2/msys2-installer/releases/download/2026-06-11/$installerName"
$installerPath = Join-Path $DownloadDir $installerName
$bash = Join-Path $MsysRoot "usr\bin\bash.exe"

if (-not (Test-Path -LiteralPath $bash)) {
    New-Item -ItemType Directory -Force -Path $DownloadDir | Out-Null

    if (-not (Test-Path -LiteralPath $installerPath)) {
        Write-Host "Downloading MSYS2 bootstrap archive..."
        Invoke-WebRequest -Uri $installerUrl -OutFile $installerPath
    }

    Write-Host "Extracting MSYS2 to $DownloadDir..."
    Start-Process -FilePath $installerPath -ArgumentList @("-y", "-o$DownloadDir") -Wait -WindowStyle Hidden
}

if (-not (Test-Path -LiteralPath $bash)) {
    throw "MSYS2 bash was not found at $bash"
}

Write-Host "Updating MSYS2 package databases and installing C CI tools..."
& $bash -lc "pacman -Syu --noconfirm --needed"
& $bash -lc "pacman -S --noconfirm --needed make pkgconf mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-clang mingw-w64-ucrt-x86_64-pkgconf mingw-w64-ucrt-x86_64-cppcheck mingw-w64-ucrt-x86_64-libsodium"

Write-Host "Verifying toolchain..."
& $bash -lc "export PATH=/ucrt64/bin:/usr/bin:`$PATH; for t in make pkg-config gcc clang cppcheck; do printf '%s: ' `$t; command -v `$t; done"

Write-Host "Local C CI environment is ready at $MsysRoot"
Write-Host "Run C CI with: powershell -ExecutionPolicy Bypass -File scripts\run-c-ci-msys2.ps1"

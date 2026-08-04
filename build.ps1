# CollatzArt Build Script (Windows)
# Windows counterpart of build.sh. Uses the CMake and MSVC toolchain that ship
# with Visual Studio 2022, so nothing extra needs to be on PATH.

$ErrorActionPreference = 'Stop'

Write-Host "Building CollatzArt..."

$root = $PSScriptRoot
$sfml = Join-Path $root 'third_party\SFML-3.0.2'

# Locate CMake: prefer one on PATH, otherwise fall back to the VS 2022 bundle.
$cmake = (Get-Command cmake -ErrorAction SilentlyContinue).Source
if (-not $cmake) {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vs = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    $cmake = Join-Path $vs 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
}
if (-not (Test-Path $cmake)) { throw "CMake not found. Install it, or install the VS 2022 'Desktop development with C++' workload." }
if (-not (Test-Path $sfml))  { throw "SFML not found at $sfml. See README for the download step." }

# Configure and build (Release)
& $cmake -S $root -B (Join-Path $root 'build') -A x64 -DCMAKE_PREFIX_PATH="$sfml"
if ($LASTEXITCODE -ne 0) { throw "Configure failed." }

& $cmake --build (Join-Path $root 'build') --config Release --parallel
if ($LASTEXITCODE -ne 0) { throw "Build failed." }

Write-Host ""
Write-Host "Build complete!"
Write-Host "To run the application:"
Write-Host "  .\build\Release\CollatzArt.exe"

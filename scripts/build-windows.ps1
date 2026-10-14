param(
    [ValidateSet("2019", "2022")]
    [string]$VisualStudio = "2019",
    [switch]$SkipInstaller
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$juce = Join-Path $root "third_party\JUCE"
$build = Join-Path $root "build"

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    throw "CMake 3.15 or newer is required."
}
if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
    throw "Git is required to obtain JUCE 7.0.12."
}
if (-not (Test-Path (Join-Path $juce "CMakeLists.txt"))) {
    New-Item -ItemType Directory -Force (Split-Path -Parent $juce) | Out-Null
    git clone --depth 1 --branch 7.0.12 https://github.com/juce-framework/JUCE.git $juce
}

if ($VisualStudio -eq "2019") {
    $generator = "Visual Studio 16 2019"
    $toolsetArgs = @()
} else {
    $generator = "Visual Studio 17 2022"
    $toolsetArgs = @("-T", "v142")
}

cmake -S $root -B $build -G $generator -A x64 @toolsetArgs `
    -DJUCE_DIR="$juce" `
    -DHERITAGE_BUILD_TESTS=ON `
    -DCMAKE_SYSTEM_VERSION=6.1
cmake --build $build --config Release --target HeritageChannel_VST3 HeritageDSPTests

& (Join-Path $build "Release\HeritageDSPTests.exe")

if (-not $SkipInstaller) {
    $makensis = Get-Command makensis.exe -ErrorAction SilentlyContinue
    if (-not $makensis) { throw "NSIS 3 is required to build the installer (makensis.exe was not found)." }
    Push-Location (Join-Path $root "installer")
    try { & $makensis.Source "HeritageChannel.nsi" } finally { Pop-Location }
}

Write-Host "Build complete."
Write-Host "VST3: $build\HeritageChannel_artefacts\Release\VST3\Heritage Channel.vst3"
if (-not $SkipInstaller) { Write-Host "Installer: $root\installer\HeritageChannel-Win64-Setup.exe" }

<#
.SYNOPSIS
    Local Windows build for vcell-mbsolver, mirroring .github/workflows/workflow.yml.

.DESCRIPTION
    Reproduces the CI "Windows" job on a developer machine:
      1. Installs the vcpkg dependencies (hdf5, curl, boost-*, pybind11) from
         vcpkg.json into ./vcpkg_installed using the x64-windows triplet.
      2. Configures CMake with the Visual Studio (MSVC) generator, wiring the
         vcpkg toolchain plus the explicit HDF5/Boost/CURL hints the project's
         CMakeLists expects under vcpkg CONFIG mode.
      3. Builds MovingBoundarySolver.exe + MovingBoundaryLib.lib (+ the pybind11
         module) in the chosen configuration.

    Requires: Visual Studio 2022 (or Build Tools) with the C++ x64 toolset,
    CMake >= 3.13, and a bootstrapped vcpkg. Strawberry Perl is needed by some
    vcpkg ports (curl/openssl) - install with `choco install strawberryperl`.

.PARAMETER VcpkgRoot
    Path to a bootstrapped vcpkg checkout. Defaults to $env:VCPKG_INSTALLATION_ROOT,
    then C:\vcpkg.

.PARAMETER Config
    CMake build configuration. Default: Release.

.PARAMETER BuildDir
    Out-of-source build directory. Default: build.

.PARAMETER Generator
    CMake generator. Defaults to auto-detecting the newest installed Visual
    Studio version (via vswhere) and falls back to "Visual Studio 17 2022" if
    detection fails. Pass this explicitly to override.

.PARAMETER Messaging
    Enable OPTION_TARGET_MESSAGING (CURL job-status messaging). Off by default,
    matching the CI Windows job.

.PARAMETER SkipVcpkg
    Skip the `vcpkg install` step (use when vcpkg_installed is already populated).

.PARAMETER Test
    Run ctest after building.

.EXAMPLE
    ./build-windows.ps1

.EXAMPLE
    ./build-windows.ps1 -Config Release -Test
#>
[CmdletBinding()]
param(
    [string]$VcpkgRoot,
    [ValidateSet('Release', 'Debug', 'RelWithDebInfo', 'MinSizeRel')]
    [string]$Config = 'Release',
    [string]$BuildDir = 'build',
    [string]$Generator,
    [switch]$Messaging,
    [switch]$SkipVcpkg,
    [switch]$Test
)

$ErrorActionPreference = 'Stop'
$repoRoot = $PSScriptRoot
Set-Location $repoRoot

# --- Pick a CMake generator matching the installed Visual Studio -----------
# windows-latest (and other Windows images) periodically ship a newer Visual
# Studio release, and CMake generator names are version-specific (e.g.
# "Visual Studio 17 2022" vs "Visual Studio 18 2026"). Auto-detect via vswhere
# instead of hard-coding a version so this keeps working as images update.
if (-not $Generator) {
    $Generator = 'Visual Studio 17 2022'
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (Test-Path $vswhere) {
        $vsVersion = & $vswhere -latest -products * -property installationVersion
        if ($vsVersion) {
            $vsMajor = [int]($vsVersion -split '\.')[0]
            $Generator = switch ($vsMajor) {
                18 { 'Visual Studio 18 2026' }
                17 { 'Visual Studio 17 2022' }
                16 { 'Visual Studio 16 2019' }
                15 { 'Visual Studio 15 2017' }
                default { 'Visual Studio 17 2022' }
            }
        }
    }
}
Write-Host "generator:  $Generator"

# --- Locate vcpkg -----------------------------------------------------------
if (-not $VcpkgRoot) {
    if ($env:VCPKG_INSTALLATION_ROOT -and (Test-Path (Join-Path $env:VCPKG_INSTALLATION_ROOT 'vcpkg.exe'))) {
        $VcpkgRoot = $env:VCPKG_INSTALLATION_ROOT
    } elseif (Test-Path 'C:\vcpkg\vcpkg.exe') {
        $VcpkgRoot = 'C:\vcpkg'
    } else {
        throw 'vcpkg.exe not found. Pass -VcpkgRoot <path> or set VCPKG_INSTALLATION_ROOT.'
    }
}
$vcpkgExe   = Join-Path $VcpkgRoot 'vcpkg.exe'
$toolchain  = Join-Path $VcpkgRoot 'scripts\buildsystems\vcpkg.cmake'
$triplet    = 'x64-windows'
$installed  = Join-Path $repoRoot 'vcpkg_installed'
$tripletDir = Join-Path $installed $triplet
Write-Host "vcpkg:      $vcpkgExe"
Write-Host "toolchain:  $toolchain"

# --- Ensure submodules ------------------------------------------------------
if (Test-Path (Join-Path $repoRoot '.gitmodules')) {
    Write-Host "`n=== Updating git submodules ===" -ForegroundColor Cyan
    git submodule update --init --recursive
}

# --- 1. Install dependencies (manifest mode) --------------------------------
if (-not $SkipVcpkg) {
    Write-Host "`n=== Installing vcpkg dependencies ($triplet) ===" -ForegroundColor Cyan
    & $vcpkgExe install --triplet $triplet
    if ($LASTEXITCODE -ne 0) { throw "vcpkg install failed ($LASTEXITCODE)" }
}

# --- 2. Configure -----------------------------------------------------------
Write-Host "`n=== Configuring CMake ($Config) ===" -ForegroundColor Cyan
$messagingFlag = if ($Messaging) { 'ON' } else { 'OFF' }
$cmakeArgs = @(
    '-S', '.', '-B', $BuildDir, '-G', $Generator, '-A', 'x64',
    "-DCMAKE_BUILD_TYPE=$Config",
    "-DCMAKE_TOOLCHAIN_FILE=$toolchain",
    "-DVCPKG_TARGET_TRIPLET=$triplet",
    "-DVCPKG_HOST_TRIPLET=$triplet",
    "-DVCPKG_INSTALLED_DIR=$installed",
    '-DVCPKG_MANIFEST_INSTALL=OFF',
    "-DCMAKE_PREFIX_PATH=$tripletDir",
    "-DBOOST_ROOT=$tripletDir",
    "-DBOOST_INCLUDEDIR=$tripletDir\include",
    "-DHDF5_ROOT=$tripletDir",
    "-DHDF5_INCLUDE_DIR=$tripletDir\include",
    "-DHDF5_CXX_INCLUDE_DIR=$tripletDir\include",
    "-DHDF5_hdf5_cpp_LIBRARY=$tripletDir\lib\hdf5_cpp.lib",
    "-DHDF5_hdf5_LIBRARY=$tripletDir\lib\hdf5.lib",
    "-DCURL_ROOT=$tripletDir",
    "-DCURL_INCLUDE_DIR=$tripletDir\include",
    "-DCURL_LIBRARY=$tripletDir\lib\libcurl.lib",
    "-DOPTION_TARGET_MESSAGING=$messagingFlag"
)
& cmake @cmakeArgs
if ($LASTEXITCODE -ne 0) { throw "CMake configure failed ($LASTEXITCODE)" }

# --- 3. Build ---------------------------------------------------------------
Write-Host "`n=== Building ($Config) ===" -ForegroundColor Cyan
& cmake --build $BuildDir --config $Config --parallel
if ($LASTEXITCODE -ne 0) { throw "Build failed ($LASTEXITCODE)" }

# --- 4. Optional tests ------------------------------------------------------
if ($Test) {
    Write-Host "`n=== Running tests ===" -ForegroundColor Cyan
    Push-Location $BuildDir
    try {
        & ctest -C $Config --output-on-failure -j2
    } finally {
        Pop-Location
    }
}

Write-Host "`n=== Done ===" -ForegroundColor Green
Write-Host "Outputs in $BuildDir\bin\$Config\ :"
Get-ChildItem (Join-Path $BuildDir "bin\$Config") -ErrorAction SilentlyContinue |
    Where-Object { $_.Name -match 'MovingBoundary' } |
    Select-Object Name, Length | Format-Table -AutoSize

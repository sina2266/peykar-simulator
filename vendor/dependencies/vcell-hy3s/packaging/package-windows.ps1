# Assemble the win64 release directory from a build tree.
#
#   pwsh packaging/package-windows.ps1 <build\bin> <out dir> <version>
#
# Run from an MSVC developer shell (dumpbin). The solvers are built against the
# static MSVC and Intel Fortran runtimes (compiler.runtime=static), so they
# should need nothing beyond Windows itself. This checks that rather than
# assuming it: every DLL they import is walked, system DLLs and API sets are
# left alone, a dynamic MSVC C++ runtime is an error (it would need a VC++
# redistributable on the user's machine), and anything else -- an Intel
# runtime DLL, say -- is found on PATH and bundled next to the executables.
param(
    [Parameter(Mandatory)] [string] $BinDir,
    [Parameter(Mandatory)] [string] $OutDir,
    [Parameter(Mandatory)] [string] $Version
)
$ErrorActionPreference = 'Stop'

$exes = 'Hybrid_EM_x64', 'Hybrid_MIL_x64', 'Hybrid_MIL_Adaptive_x64'
if (Test-Path $OutDir) { Remove-Item -Recurse -Force $OutDir }
New-Item -ItemType Directory -Force $OutDir | Out-Null

$queue = [System.Collections.Generic.Queue[string]]::new()
foreach ($e in $exes) {
    Copy-Item (Join-Path $BinDir "$e.exe") $OutDir
    $queue.Enqueue((Join-Path $OutDir "$e.exe"))
}

$system32 = Join-Path $env:WINDIR 'System32'
$seen = @{}
while ($queue.Count -gt 0) {
    $file = $queue.Dequeue()
    $imports = & dumpbin /nologo /dependents $file |
        Where-Object { $_ -match '^\s+(\S+\.dll)\s*$' } |
        ForEach-Object { $Matches[1] }
    Write-Host "$(Split-Path -Leaf $file) imports: $($imports -join ', ')"
    foreach ($dll in $imports) {
        $key = $dll.ToLowerInvariant()
        if ($seen.ContainsKey($key)) { continue }
        $seen[$key] = $true
        if ($key -match '^(api|ext)-ms-') { continue }
        if ($key -match '^(vcruntime|msvcp|concrt|vcomp)') {
            throw "${file} imports ${dll}: the dynamic MSVC runtime is linked; build with compiler.runtime=static"
        }
        if (Test-Path (Join-Path $system32 $dll)) { continue }
        $found = & where.exe $dll 2>$null | Select-Object -First 1
        if (-not $found) { throw "${file} imports ${dll}, which is neither a system DLL nor on PATH" }
        Write-Host "bundling $dll  ($found)"
        Copy-Item $found $OutDir
        $queue.Enqueue((Join-Path $OutDir $dll))
    }
}

Set-Content -Path (Join-Path $OutDir 'VERSION') -Value $Version -NoNewline:$false
$here = Split-Path -Parent $MyInvocation.MyCommand.Path
Get-Content (Join-Path $here '..\LICENSE'), (Join-Path $here 'THIRD-PARTY-NOTICES.txt') |
    Set-Content (Join-Path $OutDir 'LICENSE')
Write-Host "packaged $Version into ${OutDir}:"
Get-ChildItem $OutDir | Format-Table Name, Length

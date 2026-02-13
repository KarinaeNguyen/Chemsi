#!/usr/bin/env pwsh
# Fast incremental build - skips CMake configure unless needed
$ErrorActionPreference = "Stop"

$BUILD = "d:\Chemsi\build-mingw64"

if (-not (Test-Path $BUILD)) {
    Write-Host "First build - running full quick_build.ps1" -ForegroundColor Yellow
    & "$PSScriptRoot\cpp_engine\quick_build.ps1"
    exit $LASTEXITCODE
}

if (-not (Test-Path "$BUILD\build.ninja") -and -not (Test-Path "$BUILD\Makefile")) {
    Write-Host "Build files missing - running full quick_build.ps1" -ForegroundColor Yellow
    & "$PSScriptRoot\cpp_engine\quick_build.ps1"
    exit $LASTEXITCODE
}

Set-Location $BUILD

# Parallel build - detect which build system
$cores = (Get-CimInstance Win32_ComputerSystem).NumberOfLogicalProcessors

if (Test-Path "$BUILD\build.ninja") {
    Write-Host "Fast incremental build with Ninja ($cores jobs)..." -ForegroundColor Cyan
    $env:CMAKE_BUILD_PARALLEL_LEVEL = $cores
    & ninja.exe
} else {
    Write-Host "Fast incremental build with Make ($cores jobs)..." -ForegroundColor Cyan
    $env:CMAKE_BUILD_PARALLEL_LEVEL = $cores
    & mingw32-make.exe -j $cores
}

if ($LASTEXITCODE -ne 0) {
    Write-Host "BUILD FAILED" -ForegroundColor Red
    exit 1
}

Write-Host "=== BUILD OK ===" -ForegroundColor Green

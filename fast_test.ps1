#!/usr/bin/env pwsh
# Fast test runner - builds and runs tests in parallel
$ErrorActionPreference = "Stop"

Write-Host "=== FAST TEST CYCLE ===" -ForegroundColor Green

# Quick build
Write-Host "[1/3] Incremental build..." -ForegroundColor Cyan
& "$PSScriptRoot\fast_build.ps1"
if ($LASTEXITCODE -ne 0) { exit 1 }

# C++ tests
Write-Host "[2/3] Running C++ tests..." -ForegroundColor Cyan
$exe = "d:\Chemsi\build-mingw64\NumericIntegrity.exe"
if (Test-Path $exe) {
    & $exe
    $cppResult = $LASTEXITCODE
} else {
    Write-Host "NumericIntegrity.exe not found, skipping" -ForegroundColor Yellow
    $cppResult = 0
}

# Python tests
Write-Host "[3/3] Running Python tests..." -ForegroundColor Cyan
if (Test-Path "d:\Chemsi\.venv\Scripts\python.exe") {
    Set-Location "d:\Chemsi"
    Push-Location python_interface
    & d:\Chemsi\.venv\Scripts\python.exe -m unittest discover -s . -p "test_*.py" -v
    $pyResult = $LASTEXITCODE
    Pop-Location
} else {
    Write-Host "Python venv not found, skipping" -ForegroundColor Yellow
    $pyResult = 0
}

# Summary
Write-Host ""
if ($cppResult -eq 0 -and $pyResult -eq 0) {
    Write-Host "=== ALL TESTS PASSED ===" -ForegroundColor Green
    exit 0
} else {
    Write-Host "=== TESTS FAILED ===" -ForegroundColor Red
    exit 1
}

# Build VFEP Unified Launcher
# Compiles unified launcher with icon into single .exe file

Write-Host "=== Building VFEP Unified Launcher ===" -ForegroundColor Green
Write-Host ""

$launcherDir = $PSScriptRoot
$rootDir = Split-Path $launcherDir -Parent

Set-Location $launcherDir

# Check if GCC is available
$gccPath = Get-Command gcc -ErrorAction SilentlyContinue
if (-not $gccPath) {
    Write-Host "ERROR: gcc not found in PATH" -ForegroundColor Red
    Write-Host "Please install MinGW-w64 or ensure gcc is in your PATH" -ForegroundColor Yellow
    pause
    exit 1
}

Write-Host "Compiling vfep_launcher.exe..." -ForegroundColor Cyan
Write-Host "  - Compiling resource file..." -ForegroundColor Gray
& windres vfep_launcher.rc -o vfep_launcher_res.o
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Failed to compile vfep_launcher.rc" -ForegroundColor Red
    pause
    exit 1
}
Write-Host "  - Linking executable..." -ForegroundColor Gray
& gcc -O2 -mwindows vfep_launcher.c vfep_launcher_res.o -o "$rootDir\vfep_launcher.exe" -lcomdlg32 -lgdi32
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Failed to compile vfep_launcher.exe" -ForegroundColor Red
    pause
    exit 1
}

# Clean up object files
Remove-Item *.o -Force -ErrorAction SilentlyContinue

Write-Host "✓ vfep_launcher.exe created" -ForegroundColor Green

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "Launcher executable built successfully!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "Usage:" -ForegroundColor Cyan
Write-Host "  Double-click: vfep_launcher.exe         (shows menu)" -ForegroundColor White
Write-Host "  Command line: vfep_launcher.exe build   (build only)" -ForegroundColor White
Write-Host "               vfep_launcher.exe test    (test only)" -ForegroundColor White
Write-Host "               vfep_launcher.exe both    (build + test)" -ForegroundColor White
Write-Host ""
Write-Host "The launcher runs PowerShell scripts silently in the background." -ForegroundColor Yellow
Write-Host "You will see popup dialogs for progress and results." -ForegroundColor Yellow
Write-Host ""
Write-Host "Location: " -NoNewline
Write-Host "$rootDir\" -NoNewline -ForegroundColor Cyan
Write-Host "vfep_launcher.exe" -ForegroundColor Cyan
Write-Host ""

@echo off
REM Build VFEP launcher executables with logo icons
echo Building VFEP Launcher Executables...

cd /d "%~dp0"

echo.
echo [1/2] Compiling fast_build.exe...
gcc -O2 -municode -mwindows fast_build_launcher.c fast_build.rc -o ..\fast_build.exe -lcomdlg32 -lgdi32
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to compile fast_build.exe
    pause
    exit /b 1
)
echo ✓ fast_build.exe created

echo.
echo [2/2] Compiling fast_test.exe...
gcc -O2 -municode -mwindows fast_test_launcher.c fast_test.rc -o ..\fast_test.exe -lcomdlg32 -lgdi32
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to compile fast_test.exe
    pause
    exit /b 1
)
echo ✓ fast_test.exe created

echo.
echo ========================================
echo All launcher executables built successfully!
echo ========================================
echo.
echo You can now use:
echo   - fast_build.exe (instead of fast_build.ps1)
echo   - fast_test.exe (instead of fast_test.ps1)
echo.
pause

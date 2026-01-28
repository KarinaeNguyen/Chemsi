@echo off
echo ========================================
echo    VFEP - Running All Test Suites
echo ========================================
echo.

cd /d d:\Chemsi\build-mingw64

echo [1/2] Running Numeric Integrity Tests (40 tests)...
echo ========================================
.\NumericIntegrity.exe
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] NumericIntegrity tests FAILED!
    pause
    exit /b 1
)

echo.
echo.
echo [2/2] Running Atom Unit Tests...
echo ========================================
.\TestAtom.exe
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] TestAtom tests FAILED!
    pause
    exit /b 1
)

echo.
echo ========================================
echo    ALL TESTS PASSED SUCCESSFULLY!
echo ========================================
echo.
echo Test Summary:
echo   - NumericIntegrity: 40/40 tests PASS
echo   - TestAtom:         All tests PASS
echo.

pause

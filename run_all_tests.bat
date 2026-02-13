@echo off
echo ========================================
echo    VFEP - Running All Test Suites
echo ========================================
echo.

cd /d d:\Chemsi\build-mingw64

echo [1/3] Running Numeric Integrity Tests...
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
echo [2/3] Running Atom Unit Tests...
echo ========================================
.\TestAtom.exe
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] TestAtom tests FAILED!
    pause
    exit /b 1
)

echo.
echo.
echo [3/3] Running Surrogate Model Tests (Phase 9E)...
echo ========================================
cd /d d:\Chemsi
"d:\Chemsi\.venv\Scripts\python.exe" -m unittest python_interface.test_surrogate_model
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Surrogate model tests FAILED!
    pause
    exit /b 1
)

echo.
echo ========================================
echo    ALL TESTS PASSED SUCCESSFULLY!
echo ========================================
echo.
echo Test Summary:
echo   - NumericIntegrity: PASS
echo   - TestAtom:         PASS
echo   - SurrogateModel:   PASS
echo.

pause

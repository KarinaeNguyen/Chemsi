@echo off
setlocal

set "MSYS=C:\msys64"
set "PROJ=/d/Chemsi/cpp_engine"
set "BUILD=build-mingw64"

"%MSYS%\mingw64.exe" -defterm -here -no-start -lc ^
"set -e; \
cd '%PROJ%'; \
rm -rf '%BUILD%'; \
cmake -S . -B '%BUILD%' -G 'MinGW Makefiles' -DCMAKE_BUILD_TYPE=Release -DCHEMSI_BUILD_VIS=ON; \
cmake --build '%BUILD%' -j; \
echo; echo ===== Running Numeric Integrity Gate =====; \
./'%BUILD%'/NumericIntegrity.exe; \
echo; echo ===== Launching Visualizer (Calibration) =====; \
export PATH=/c/msys64/mingw64/bin:$PATH; \
./'%BUILD%'/VFEP.exe --calib"

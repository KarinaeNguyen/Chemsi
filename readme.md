CHEMSI/
├─ cpp_engine/                      # Core C++ engine (simulation + tools + visualization)
│  ├─ CMakeLists.txt                # CMake entry point for the C++ engine
│  ├─ include/                      # Public headers / engine-facing APIs
│  │  └─ Simulation.h               # Simulation API + Observation types (hotspot_pos_m_* etc.)
│  ├─ src/                          # Engine implementation (simulation core)
│  │  ├─ Simulation.cpp             # Simulation core implementation (step(), observe(), etc.)
│  │  └─ GrpcSimServer.cpp          # gRPC server bridge (simulation <-> external clients)
│  ├─ vis/                          # Visualization app (ImGui/ImPlot + OpenGL)
│  │  └─ main_vis.cpp               # Visualizer entry point (current UI + rendering)
│  ├─ world/                        # World/model primitives (NO UI dependencies)
│  │  └─ ceiling_rail.h             # Step 1: first-class ceiling rail interface (room-attached)
│  ├─ tests/                        # C++ tests (unit/integration)
│  ├─ Testing/                      # Test harness assets/config (if used by your build)
│  ├─ det_runs/                     # Deterministic run outputs/logs (if used)
│  ├─ build_release/                # Local build output (not source-controlled)
│  ├─ build-release/                # Local build output (not source-controlled)
│  ├─ launch_all.sh                 # Convenience runner (POSIX)
│  └─ launch_all.bat                # Convenience runner (Windows)
│
├─ proto/                           # Protobuf/gRPC IDL definitions
├─ python_interface/                # Python integration/tools (clients, scripts, wrappers)
├─ third_party/                     # External dependencies (vendored/submodules)
├─ Testing/                         # Top-level test data/tools (if used)
│
├─ build_msys/                      # Local build output (MSYS) — not source-controlled
├─ build-mingw64/                   # Local build output (MinGW64) — not source-controlled
│
├─ readme.md                        # Project overview (this file)
├─ PROGRESS.md                      # Progress log / milestone notes
├─ architect.txt                    # Architecture notes
├─ launch_all.sh                    # Top-level convenience script (if used)
├─ imgui.ini                        # Local UI layout state (usually ignored)
└─ *.log / *.txt / *.csv            # Local diagnostics + run artifacts (configure.log, cmake_trace, etc.)

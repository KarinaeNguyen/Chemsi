#include <iostream>
#include <fstream>
#include <iomanip>
#include <cctype>
#include <string>
#include <vector>
#include <filesystem>
#include <cstdlib>
#include "Simulation.h"
#include "ObjectModel.h"
#include "GrpcSimServer.h"

#if defined(_WIN32)
  #include <conio.h>
  static bool keyPressed() { return _kbhit() != 0; }
  static char readKey() { return (char)_getch(); }
#else
  static bool keyPressed() { return false; }
  static char readKey() { return 0; }
#endif

namespace {
    static void printUsage(const char* argv0) {
        std::cout
            << "Usage: " << argv0 << " [options]\n\n"
            << "Options:\n"
            << "  --dt <seconds>            Integration timestep (default: 0.02)\n"
            << "  --t_end <seconds>         Hard stop time (default: 60)\n"
            << "  --log_dt <seconds>        CSV logging interval (default: 0.10)\n"
            << "  --out <path>              CSV output path (default: high_fidelity_ml.csv)\n"
            << "  --ignite_at <seconds>     Auto-ignite/increase pyrolysis at time (default: 2.0)\n"
            << "  --suppress_at <seconds>   Auto-start suppression at time (default: 5.0)\n"
            << "  --no_auto                 Disable auto ignition/suppression (Windows keys remain)\n"
            << "  --init_objects             Instantiate default VFEP object store and validate (exits)\n"
            << "  --dump_objects <path>      Write object-store summary/validation report to a file (implies --init_objects)\n"
            << "  --grpc_port <port>         Start gRPC server for Unity integration (default: disabled)\n"
            << "  --grpc_bind <addr>         Bind address for gRPC server (default: 127.0.0.1)\n"
            << "  --tick_hz <N>              Fixed update rate when gRPC is enabled (default: 20)\n"
            << "  -h, --help                Show this help\n\n"
            << "Windows interactive keys (when available): F=ignite/increase pyrolysis | S=start suppression | Q=quit\n";
    }

    static bool parseDouble(const std::string& s, double& out) {
        char* end = nullptr;
        const double v = std::strtod(s.c_str(), &end);
        if (end == s.c_str() || (end && *end != '\0')) return false;
        out = v;
        return true;
    }

    static void ensureParentDirExists(const std::filesystem::path& p) {
        const auto parent = p.parent_path();
        if (!parent.empty()) {
            std::error_code ec;
            std::filesystem::create_directories(parent, ec);
        }
    }
}

int main(int argc, char** argv) {
    std::cout << "=== CHEMSI VFEP: Stoichiometry + Thermodynamics + Ventilation + Suppression + Li-ion Runaway ===\n";

    // ----------------------------
    // Defaults (launch-safe)
    // ----------------------------
    double dt = 0.02;
    double t_end = 60.0;
    double log_dt = 0.10;
    std::filesystem::path outPath = "high_fidelity_ml.csv";
    double ignite_at = 2.0;
    double suppress_at = 5.0;
    bool autoActions = true;

    bool initObjects = false;
    std::filesystem::path dumpObjectsPath;

    // gRPC (Unity integration)
    int grpc_port = 0;
    std::string grpc_bind = "127.0.0.1";
    int tick_hz = 20;

    // ----------------------------
    // CLI parsing
    // ----------------------------
    std::vector<std::string> args(argv + 1, argv + argc);
    for (size_t i = 0; i < args.size(); ++i) {
        const std::string& a = args[i];
        if (a == "-h" || a == "--help") {
            printUsage(argv[0]);
            return 0;
        }
        if (a == "--no_auto") {
            autoActions = false;
            continue;
        }
        if (a == "--init_objects") {
            initObjects = true;
            continue;
        }

        auto needValue = [&](double& target) -> bool {
            if (i + 1 >= args.size()) return false;
            double v = 0.0;
            if (!parseDouble(args[i + 1], v)) return false;
            target = v;
            ++i;
            return true;
        };

        if (a == "--dt") {
            if (!needValue(dt) || dt <= 0.0) { std::cerr << "Invalid --dt\n"; return 2; }
        } else if (a == "--t_end") {
            if (!needValue(t_end) || t_end <= 0.0) { std::cerr << "Invalid --t_end\n"; return 2; }
        } else if (a == "--log_dt") {
            if (!needValue(log_dt) || log_dt <= 0.0) { std::cerr << "Invalid --log_dt\n"; return 2; }
        } else if (a == "--ignite_at") {
            if (!needValue(ignite_at) || ignite_at < 0.0) { std::cerr << "Invalid --ignite_at\n"; return 2; }
        } else if (a == "--suppress_at") {
            if (!needValue(suppress_at) || suppress_at < 0.0) { std::cerr << "Invalid --suppress_at\n"; return 2; }

} else if (a == "--tick_hz") {
    if (i + 1 >= args.size()) { std::cerr << "Missing value for --tick_hz\n"; return 2; }
    tick_hz = std::atoi(args[i + 1].c_str());
    if (tick_hz <= 0) { std::cerr << "Invalid --tick_hz\n"; return 2; }
    ++i;
} else if (a == "--grpc_port") {
    if (i + 1 >= args.size()) { std::cerr << "Missing value for --grpc_port\n"; return 2; }
    grpc_port = std::atoi(args[i + 1].c_str());
    if (grpc_port <= 0 || grpc_port > 65535) { std::cerr << "Invalid --grpc_port\n"; return 2; }
    ++i;
} else if (a == "--grpc_bind") {
    if (i + 1 >= args.size()) { std::cerr << "Missing value for --grpc_bind\n"; return 2; }
    grpc_bind = args[i + 1];
    ++i;
        } else if (a == "--out") {
            if (i + 1 >= args.size()) { std::cerr << "Missing value for --out\n"; return 2; }
            outPath = args[i + 1];
            ++i;
        } else if (a == "--dump_objects") {
            if (i + 1 >= args.size()) { std::cerr << "Missing value for --dump_objects\n"; return 2; }
            dumpObjectsPath = args[i + 1];
            initObjects = true;
            ++i;
        } else {
            std::cerr << "Unknown option: " << a << "\n";
            printUsage(argv[0]);
            return 2;
        }
    }

    if (initObjects) {
        auto store = vfep::obj::makeDefault4x4ObjectStore();
        const auto report = store.validate();

        auto emit = [&](std::ostream& os) {
            os << "VFEP Object Store (default 4x4)\n";
            os << "--------------------------------\n";
            os << "Rooms:            " << store.rooms.size() << "\n";
            os << "Racks:            " << store.racks.size() << "\n";
            os << "VFEPS:            " << store.vfeps.size() << "\n";
            os << "Sensors:          " << store.sensors.size() << "\n";
            os << "Rails:            " << store.rails.size() << "\n";
            os << "Tanks:            " << store.tanks.size() << "\n";
            os << "Arms:             " << store.arms.size() << "\n";
            os << "Nozzles:          " << store.nozzles.size() << "\n";
            os << "Magazines:        " << store.magazines.size() << "\n";
            os << "TriggerConfigs:   " << store.trigger_configs.size() << "\n";
            os << "ManualControls:   " << store.manual_controls.size() << "\n";
            os << "VFE Round Types:  " << store.vfe_round_types.size() << "\n";
            os << "Chemicals:        " << store.chemicals.size() << "\n";
            os << "Shells:           " << store.shells.size() << "\n";
            os << "Cameras:          " << store.cameras.size() << "\n";
            os << "ArmViews:         " << store.arm_views.size() << "\n";
            os << "DataCenterViews:  " << store.datacenter_views.size() << "\n";
            os << "Maps:             " << store.maps.size() << "\n\n";

            os << "Validation: " << (report.ok() ? "OK" : "FAILED") << "\n";
            os << "Issues: " << report.issues.size() << "\n";
            for (const auto& iss : report.issues) {
                os << " - [" << (iss.severity == vfep::obj::ValidationIssue::Severity::Error ? "ERROR" : "WARN")
                   << "] " << iss.object_type << ":" << iss.object_id << " - " << iss.message << "\n";
            }
        };

        emit(std::cout);

        if (!dumpObjectsPath.empty()) {
            ensureParentDirExists(dumpObjectsPath);
            std::ofstream out(dumpObjectsPath);
            if (!out) {
                std::cerr << "ERROR: Could not open --dump_objects path: " << dumpObjectsPath.string() << "\n";
                return 2;
            }
            emit(out);
            out.close();
            std::cout << "Report written to " << dumpObjectsPath.string() << "\n";
        }

        if (grpc_port == 0) {
            return report.ok() ? 0 : 3;
        }
        // If gRPC is requested, continue into server mode after initialization/validation.

    }


if (grpc_port != 0) {
    // gRPC server mode: fixed timestep object-store mechanics loop + telemetry streaming.
    // This mode is separate from the high-fidelity chemical/ventilation simulation.
    vfep::grpcsim::GrpcSimServer server;
    const bool ok = server.Run(grpc_bind, grpc_port, tick_hz);
    return ok ? 0 : 4;
}

    if (autoActions) {
        std::cout << "Auto-actions: ignite_at=" << ignite_at << "s, suppress_at=" << suppress_at
                  << "s (use --no_auto to disable)\n";
    }
    std::cout << "Run controls: dt=" << dt << "s, t_end=" << t_end << "s, log_dt=" << log_dt
              << "s, out=" << outPath.string() << "\n";

    vfep::Simulation sim;
    sim.resetToDataCenterRackScenario();

    ensureParentDirExists(outPath);
    std::ofstream csv(outPath);
    csv << "time_s,T_K,HRR_kW,O2_volpct,CO2_volpct,H2O_volpct,fuel_kg,inhib_kgm3,inert_kgm3,ACH,agent_mdot_kgps,reward\n";
    csv << std::fixed << std::setprecision(6);

    double t = 0.0;
    double next_log_t = 0.0;
    double next_status_t = 0.0;

    bool didIgnite = false;
    bool didSuppress = false;

    while (true) {
        if (keyPressed()) {
            char k = (char)std::toupper(readKey());
            if (k == 'Q') break;
            if (k == 'F') sim.commandIgniteOrIncreasePyrolysis();
            if (k == 'S') sim.commandStartSuppression();
        }

        // Launch-safe default behavior (portable): scheduled ignition/suppression.
        if (autoActions) {
            if (!didIgnite && t >= ignite_at) {
                sim.commandIgniteOrIncreasePyrolysis();
                didIgnite = true;
            }
            if (!didSuppress && t >= suppress_at) {
                sim.commandStartSuppression();
                didSuppress = true;
            }
        }

        sim.step(dt);
        t += dt;

        const auto o = sim.observe();

        if (t >= next_log_t) {
            csv << t << "," << o.T_K << "," << (o.HRR_W / 1000.0) << ","
                << o.O2_volpct << "," << o.CO2_volpct << "," << o.H2O_volpct << ","
                << o.fuel_kg << "," << o.inhibitor_kgm3 << "," << o.inert_kgm3 << ","
                << o.ACH << "," << o.agent_mdot_kgps << "," << o.reward << "\n";
            next_log_t += log_dt;
        }

        if (t >= next_status_t) {
            std::cout << "\r"
                      << "t=" << std::setw(7) << std::setprecision(2) << t << " s | "
                      << "T=" << std::setw(7) << std::setprecision(1) << (o.T_K - 273.15) << " C | "
                      << "HRR=" << std::setw(7) << std::setprecision(1) << (o.HRR_W / 1000.0) << " kW | "
                      << "O2=" << std::setw(6) << std::setprecision(2) << o.O2_volpct << "% | "
                      << "Fuel=" << std::setw(7) << std::setprecision(3) << o.fuel_kg << " kg | "
                      << "Agent=" << std::setw(6) << std::setprecision(3) << o.agent_mdot_kgps << " kg/s   "
                      << std::flush;
            next_status_t += 0.5;
        }

        if (sim.isConcluded()) {
            std::cout << "\n\nSimulation ended: extinguished + cooled.\n";
            break;
        }

        if (t >= t_end) {
            std::cout << "\n\nSimulation ended: reached hard stop t_end=" << t_end << " s.\n";
            break;
        }
    }

    csv.close();
    std::cout << "\nCSV written to " << outPath.string() << "\n";
    return 0;
}

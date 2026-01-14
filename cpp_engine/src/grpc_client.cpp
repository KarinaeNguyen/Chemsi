#include <iostream>
#include <string>
#include <thread>

#ifdef CHEMSI_ENABLE_GRPC
#include <grpcpp/grpcpp.h>
#include "vfep_sim_service_v1.grpc.pb.h"
#endif

static void usage(const char* exe) {
    std::cout << "Usage: " << exe << " --addr <host:port> [--frames N]\n";
}

int main(int argc, char** argv) {
    std::string addr = "127.0.0.1:50051";
    int frames = 10;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--addr" && i + 1 < argc) { addr = argv[++i]; continue; }
        if (a == "--frames" && i + 1 < argc) { frames = std::atoi(argv[++i]); continue; }
        if (a == "-h" || a == "--help") { usage(argv[0]); return 0; }
    }

#ifndef CHEMSI_ENABLE_GRPC
    std::cerr << "CHEMSI_ENABLE_GRPC not enabled.\n";
    return 2;
#else
    using chemsi::vfep::v1::VFEPUnitySimServiceV1;
    using chemsi::vfep::v1::EmptyV1;
    using chemsi::vfep::v1::TelemetryFrameV1;

    auto channel = grpc::CreateChannel(addr, grpc::InsecureChannelCredentials());
    auto stub = VFEPUnitySimServiceV1::NewStub(channel);

    // Snapshot
    {
        grpc::ClientContext ctx;
        EmptyV1 req;
        chemsi::vfep::v1::WorldSnapshotV1 snap;
        const auto st = stub->GetWorldSnapshot(&ctx, req, &snap);
        if (!st.ok()) {
            std::cerr << "GetWorldSnapshot failed: " << st.error_message() << "\n";
            return 3;
        }
        std::cout << "Snapshot schema=" << snap.schema_version()
                  << " rooms=" << snap.rooms_size()
                  << " racks=" << snap.racks_size()
                  << " vfeps=" << snap.vfeps_size()
                  << " arms=" << snap.arms_size()
                  << " nozzles=" << snap.nozzles_size()
                  << " tanks=" << snap.tanks_size()
                  << "\n";
    }

    // Telemetry stream (read N frames)
    {
        grpc::ClientContext ctx;
        EmptyV1 req;
        auto reader = stub->StreamTelemetry(&ctx, req);
        TelemetryFrameV1 frame;
        int count = 0;
        while (count < frames && reader->Read(&frame)) {
            std::cout << "Frame t=" << frame.sim_time_s()
                      << " racks=" << frame.racks_size()
                      << " tanks=" << frame.tanks_size()
                      << " arms=" << frame.arms_size()
                      << " nozzles=" << frame.nozzles_size()
                      << " suppression=" << (frame.vfeps_size() ? frame.vfeps(0).suppression_active() : false)
                      << "\n";
            ++count;
        }
        auto st = reader->Finish();
        if (!st.ok()) {
            std::cerr << "StreamTelemetry ended with error: " << st.error_message() << "\n";
            return 4;
        }
        std::cout << "Read " << count << " frames.\n";
    }

    return 0;
#endif
}

#pragma once

#include "ObjectModel.h"
#include "MechanicsSim.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

namespace vfep {
namespace grpcsim {

// Simple synchronous gRPC server wrapper.
// Intended for Unity integration (Option B: gRPC + Protobuf) and streaming telemetry.
class GrpcSimServer {
public:
    GrpcSimServer();
    ~GrpcSimServer();

    // Starts the server and a fixed-timestep sim thread.
    // Blocks until Stop() is called (or process exits).
    bool Run(const std::string& bind_addr, int port, int tick_hz);

    void Stop();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace grpcsim
} // namespace vfep

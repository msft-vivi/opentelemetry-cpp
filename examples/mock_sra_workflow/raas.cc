#include "client/raas_client.h"
#include "server/raas_server.h"
#include "tracer_common.h"
#include "utilities.h"

#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <future>

namespace trace_api = opentelemetry::trace;

void RunRaaSServer()
{
    std::string ip = "0.0.0.0";
    uint16_t port = 8801;
    std::string endPoint = ip + ":" +  std::to_string(port);

    sra::RaasServer service;
    grpc::ServerBuilder builder;

    builder.RegisterService(&service);
    builder.AddListeningPort(endPoint, grpc::InsecureServerCredentials());
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "RaasServer listening on port: " << endPoint << std::endl;

    server->Wait();
    server->Shutdown();
}

int main(int argc, char** argv)
{
    sra::InitRaaSTracer();
    RunRaaSServer();
    sra::CleanupTracer();

    return 0;
}
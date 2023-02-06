#include "client/saas_client.h"
#include "server/saas_server.h"
#include "tracer_common.h"
#include "utilities.h"

#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <future>

namespace trace_api = opentelemetry::trace;

void RunSaaSServer()
{
    std::string ip = "0.0.0.0";
    uint16_t port = 8802;
    std::string endPoint = ip + ":" +  std::to_string(port);

    sra::SaasServer service;
    grpc::ServerBuilder builder;

    builder.RegisterService(&service);
    builder.AddListeningPort(endPoint, grpc::InsecureServerCredentials());
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "SaasServer listening on port: " << endPoint << std::endl;

    server->Wait();
    server->Shutdown();
}

int main(int argc, char** argv)
{
    sra::InitSaaSTracer();
    RunSaaSServer();
    sra::CleanupTracer();

    return 0;
}
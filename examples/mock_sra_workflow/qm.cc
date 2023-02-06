#include "client/query_master_client.h"
#include "server/query_master_server.h"
#include "client/raas_client.h"
#include "server/raas_server.h"
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

void RunQueryMasterServer()
{
    std::string ip = "0.0.0.0";
    uint16_t port = 8800;
    std::string endPoint = ip + ":" +  std::to_string(port);

    sra::QueryMasterServer service;
    grpc::ServerBuilder builder;

    builder.RegisterService(&service);
    builder.AddListeningPort(endPoint, grpc::InsecureServerCredentials());
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "QueryMasterServer listening on port: " << endPoint << std::endl;

    server->Wait();
    server->Shutdown();
}

int main(int argc, char** argv)
{
    sra::InitQueryMasterTracer();
    RunQueryMasterServer();
    sra::CleanupTracer();

    return 0;
}
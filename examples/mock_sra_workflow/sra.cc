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

// std::mutex cv_m;
// std::condition_variable cv;

namespace trace_api = opentelemetry::trace;

namespace sra
{
    const char* c_sraLibName = "SraLib";
    const char* c_queryMasterClientLibName = "QueryMasterClientLib";
    const char* c_queryMasterServerLibName = "QueryMasterServerLib";
    const char* c_raasClientLibName = "RaaSClientLib";
    const char* c_raasServerLibName = "RaaSServerLib";
    const char* c_saasClientLibName = "SaaSClientLib";
    const char* c_saasServerLibName = "SaaSServerLib";
    const char* c_versionNumber = "1.0.0";
}

// IF RunQueryMasterClient call always failed, check whether some executable has take that port.
void RunQueryMasterClient()
{
    std::string ip = "0.0.0.0";
    uint16_t port = 8800;

    // Build rpc channel
    std::string endPoint = ip + ":" +  std::to_string(port);
    auto queryMasterChannel = grpc::CreateChannel(endPoint, grpc::InsecureChannelCredentials());

    // Build QueryMasterClien and kick off rpc
    sra::QueryMasterClient queryMasterClient (queryMasterChannel);
    
    // // Wait for QueryMasterServer start.
    // std::unique_lock<std::mutex> lk {cv_m};
    // cv.wait(lk);

    // Kick off call
    bool success = queryMasterClient.call(ip, port);
    if (success)
    {
        std::cout << "Call query master server success." << std::endl;
    }
    else
    {
        std::cout << "Call query master server failed." << std::endl;
    }
}

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

    // // Signal to awake client
    // cv.notify_one();

    server->Wait();
    server->Shutdown();
}

void RunRaaSClient()
{
    std::string ip = "0.0.0.0";
    uint16_t port = 8801;

    // Build rpc channel
    std::string endPoint = ip + ":" +  std::to_string(port);
    auto raasChannel = grpc::CreateChannel(endPoint, grpc::InsecureChannelCredentials());

    // Build RaasClient and kick off rpc
    sra::RaaSClient raasClient (raasChannel);

    // Kick off call
    bool success = raasClient.call(ip, port);
    if (success)
    {
        std::cout << "Call raas server success." << std::endl;
    }
    else
    {
        std::cout << "Call raas server failed." << std::endl;
    }
}

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

void RunSaaSClient()
{
    std::string ip = "0.0.0.0";
    uint16_t port = 8802;

    // Build rpc channel
    std::string endPoint = ip + ":" +  std::to_string(port);
    auto saasChannel = grpc::CreateChannel(endPoint, grpc::InsecureChannelCredentials());

    // Build RaasClient and kick off rpc
    sra::SaaSClient saasClient (saasChannel);

    // Kick off call
    bool success = saasClient.call(ip, port);
    if (success)
    {
        std::cout << "Call saas server success." << std::endl;
    }
    else
    {
        std::cout << "Call saas server failed." << std::endl;
    }
}

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
    InitTracer();
    // set global propagator
    opentelemetry::context::propagation::GlobalTextMapPropagator::SetGlobalPropagator(
        opentelemetry::nostd::shared_ptr<opentelemetry::context::propagation::TextMapPropagator>(
            new trace_api::propagation::HttpTraceContext()));

    std::string rootSpanName = "Root_Span";
    trace_api::StartSpanOptions opts;
    opts.kind = trace_api::SpanKind::kClient;
    auto rootSpan = get_tracer(sra::c_sraLibName, sra::c_versionNumber)->StartSpan(rootSpanName, {}, opts);
    auto scope = get_tracer(sra::c_sraLibName)->WithActiveSpan(rootSpan);

    // std::thread th1(&RunQueryMasterServer);
    // std::thread th2(&RunQueryMasterClient);
    // th2.join();
    auto f1 = std::async(std::launch::async, RunQueryMasterServer);
    auto f2 = std::async(std::launch::async, RunRaaSServer);
    auto f3 = std::async(std::launch::async, RunSaaSServer);
    // auto f2 = std::async(std::launch::deferred, RunQueryMasterClient);
    // std::this_thread::sleep_for(std::chrono::seconds(3));
    // f2.get();

    RunQueryMasterClient();
    RunRaaSClient();
    RunSaaSClient();

    rootSpan->SetStatus(trace_api::StatusCode::kOk);
    rootSpan->SetAttribute("MachineName", "SraMachine");
    rootSpan->End();

    CleanupTracer();
    std::this_thread::sleep_for(std::chrono::minutes(10));

    return 0;
}
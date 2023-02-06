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

int main(int argc, char** argv)
{
    sra::InitSraTracer();

    std::string rootSpanName = "Root_Span";
    trace_api::StartSpanOptions opts;
    opts.kind = trace_api::SpanKind::kClient;
    auto rootSpan = sra::get_tracer(sra::c_sraLibName, sra::c_versionNumber)->StartSpan(rootSpanName, {}, opts);
    auto scope = sra::get_tracer(sra::c_sraLibName)->WithActiveSpan(rootSpan);

    RunQueryMasterClient();
    RunRaaSClient();
    RunSaaSClient();

    rootSpan->SetStatus(trace_api::StatusCode::kOk);
    rootSpan->SetAttribute("MachineName", "SraMachine");
    rootSpan->End();

    sra::CleanupTracer();

    return 0;
}
#include "messages.grpc.pb.h"

#include <grpcpp/grpcpp.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include "../tracer_common.h"

#include <chrono>
#include <thread>
#include <map>
#include <string>

using sra_demo::RaaS;
using sra_demo::RaaSRequest;
using sra_demo::RaaSResponse;

namespace sra
{

namespace trace_api = opentelemetry::trace;
using RpcHeaderT = std::map<std::string, std::string>;
extern const char* c_raasServerLibName;
extern const char* c_versionNumber;

class RaasServer final : public RaaS::Service
{
public:
    grpc::Status CallService(grpc::ServerContext* context,
                       const RaaSRequest* request,
                       RaaSResponse* response) override
    {
        response->set_m_message("This is a response message from RaaS Server.");

        // Create a SpanOptions object and set the kind to Server to inform OpenTel.
        trace_api::StartSpanOptions options;
        options.kind = trace_api::SpanKind::kServer;

        RpcHeaderT remoteKeyValuePairs {request->m_spancontext().begin(), request->m_spancontext().end()};

        // For debug.
        #ifdef LOG_RPC_HEADER
            LogRpcHeader(remoteKeyValuePairs);
        #endif

        const BondRpcTextMapCarrier<RpcHeaderT> carrier {&remoteKeyValuePairs};
        auto prop        = opentelemetry::context::propagation::GlobalTextMapPropagator::GetGlobalPropagator();
        auto current_ctx = opentelemetry::context::RuntimeContext::GetCurrent();
        auto new_context = prop->Extract(carrier, current_ctx);
        options.parent   = trace_api::GetSpan(new_context)->GetContext();

        std::string spanName = "Raas_Server_Span";
        auto span = get_tracer(c_raasServerLibName, c_versionNumber)->StartSpan(spanName, {}, options);
        span->SetStatus(trace_api::StatusCode::kOk);
        span->SetAttribute("MachineName", "RaaSServerMachine");

        #ifdef ENABLE_SLEEP
            // Sleep for a while
            std::this_thread::sleep_for(std::chrono::seconds(1));
        #endif
        span->End();

        return grpc::Status::OK;
    }

    void LogRpcHeader(RpcHeaderT p_headers)
    {
        std::cout << "Rpc headers: " << std:: endl;
        for (const auto& elem : p_headers)
        {
            std::cout << elem.first << ": " << elem.second << std::endl;
        }
    }
};

} // namespace

#include <grpcpp/grpcpp.h>
#include "messages.grpc.pb.h"

#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <chrono>

#include "opentelemetry/trace/semantic_conventions.h"
#include "opentelemetry/nostd/string_view.h"
#include "../utilities.h"
#include "../tracer_common.h"

using sra_demo::RaaSRequest;
using sra_demo::RaaSResponse;
using sra_demo::RaaS;

namespace sra
{

namespace trace_api = opentelemetry::trace;
using RpcHeaderT = std::map<std::string, std::string>;
extern const char* c_raasClientLibName;
extern const char* c_versionNumber;

class RaaSClient
{
public:
    RaaSClient(std::shared_ptr<grpc::Channel> channel) : m_stub(RaaS::NewStub(channel)) {}
    bool call(std::string ip, uint16_t port)
    {
        RaaSRequest request;
        RaaSResponse response;

        trace_api::StartSpanOptions options;
        options.kind = trace_api::SpanKind::kClient;

        #ifdef ENABLE_SLEEP
            // Sleep for a while.
            std::this_thread::sleep_for(std::chrono::seconds(2));
        #endif

        std::string spanName = "RaaS_Client_Span";
        auto span = get_tracer(c_raasClientLibName, c_versionNumber)->StartSpan(
                                                spanName,
                                                {},
                                                options);
        #ifdef ENABLE_SLEEP
            // Sleep for a while.
            std::this_thread::sleep_for(std::chrono::seconds(1));
        #endif

        auto scope = get_tracer(c_raasClientLibName, c_versionNumber)->WithActiveSpan(span);

        // inject current context to grpc metadata
        auto current_ctx = opentelemetry::context::RuntimeContext::GetCurrent();
        RpcHeaderT attributes;
        BondRpcTextMapCarrier<RpcHeaderT> carrier {&attributes};
        auto prop = opentelemetry::context::propagation::GlobalTextMapPropagator::GetGlobalPropagator();
        prop->Inject(carrier, current_ctx);

        // Shallow copy
        for(const auto& pair : *carrier.m_headers)
        {
            (*request.mutable_m_spancontext())[pair.first] = pair.second;
        }

        auto callService = [&]()
            {
                grpc::ClientContext context;
                grpc::Status status = m_stub->CallService(&context, request, &response);
                return status.ok();
            };

        bool isSuccess = RetryUtility::RunOnFailureRetry(callService, 3, 2);

        if (isSuccess)
        {
            span->SetStatus(trace_api::StatusCode::kOk);
            span->SetAttribute("MachineName", "SraMachine-RaaS-Client");
            // span->SetAttribute(trace_api::SemanticConventions::kRpcGrpcStatusCode, status.error_code());
            span->End();
            return true;
        }
        else
        {
            span->SetStatus(trace_api::StatusCode::kError);
            span->SetAttribute("MachineName", "SraMachine-RaaS-Client");
            // span->SetAttribute(trace_api::SemanticConventions::kRpcGrpcStatusCode, status.error_code());
            span->End();
            return false;
        }
    }

private:
    std::unique_ptr<RaaS::Stub> m_stub;
};

} // namespace

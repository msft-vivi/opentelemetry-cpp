#ifdef BAZEL_BUILD
#  include "examples/grpc/protos/messages.grpc.pb.h"
#else
#  include "messages.grpc.pb.h"
#endif

#define LOG_RPC_HEADER

#include "opentelemetry/trace/context.h"
#include "opentelemetry/trace/semantic_conventions.h"
#include "opentelemetry/trace/span_context_kv_iterable_view.h"
#include "tracer_common.h"

#include <grpcpp/grpcpp.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include <chrono>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <chrono>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;

using grpc_example::Greeter;
using grpc_example::GreetRequest;
using grpc_example::GreetResponse;

using Span        = opentelemetry::trace::Span;
using SpanContext = opentelemetry::trace::SpanContext;
using namespace opentelemetry::trace;

namespace context = opentelemetry::context;

namespace
{
using RpcHeaderT = std::map<std::string, std::string>;

class GreeterServer final : public Greeter::Service
{
public:
  Status Greet(ServerContext *context,
               const GreetRequest *request,
               GreetResponse *response) override
  {
    // Sleep for 5s to make distinguished the hierarchial relationship between parent span and child span.
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Create a SpanOptions object and set the kind to Server to inform OpenTel.
    StartSpanOptions options;
    options.kind = SpanKind::kServer;

    // extract context from grpc message
    //FIXME: How to debug rpc call ? eg: carrier is nullptr that caused segmentation falut.
    RpcHeaderT remoteKeyValuePairs {request->m_spancontext().begin(), request->m_spancontext().end()};

    // For debug.
    #ifdef LOG_RPC_HEADER
        LogRpcHeader(remoteKeyValuePairs);
    #endif

    BondRpcTextMapCarrier<RpcHeaderT> carrier {&remoteKeyValuePairs};
    auto prop = context::propagation::GlobalTextMapPropagator::GetGlobalPropagator();

    // --------------------- Server-Side---------------------------
    // Extract context from carrier
    auto current_ctx = context::RuntimeContext::GetCurrent();
    auto new_context = prop->Extract(carrier, current_ctx);
    options.parent   = GetSpan(new_context)->GetContext();

    // Create a child span
    auto spanName = "ServerSpan";
    auto childSpan = get_tracer("grpc_server_lib")
        ->StartSpan(spanName,
                    {{SemanticConventions::kRpcSystem, "grpc"},
                    {SemanticConventions::kRpcService, "GreeterService"},
                    {SemanticConventions::kRpcMethod, "Greet"},
                    {SemanticConventions::kRpcGrpcStatusCode, 0}},
                    options);

    // Make child span to be the active span.
    auto scope = get_tracer("grpc_server_lib")->WithActiveSpan(childSpan);

    // Mock some operations.
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Fetch and parse whatever HTTP headers we can from the gRPC request.
    // childSpan->AddEvent("Processing client attributes");

    std::string req = request->m_message();

    // Send response to client
    response->set_m_message("This is a response message.");
    // childSpan->AddEvent("Response sent to client");
    childSpan->SetStatus(StatusCode::kOk);
    // Make sure to end your spans!
    childSpan->End();
    return Status::OK;
  }

  
  void LogRpcHeader(RpcHeaderT p_headers)
  {
    std::cout << "Rpc headers: " << std:: endl;
    for (const auto& elem : p_headers)
    {
        std::cout << elem.first << ": " << elem.second << std::endl;
    }
  }
};  // GreeterServer class

void RunServer(uint16_t port)
{
  std::string address("0.0.0.0:" + std::to_string(port));
  GreeterServer service;
  ServerBuilder builder;

  builder.RegisterService(&service);
  builder.AddListeningPort(address, grpc::InsecureServerCredentials());

  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on port: " << address << std::endl;
  server->Wait();
  server->Shutdown();
}
}  // namespace

int main(int argc, char **argv)
{
  InitServerTracer();
  constexpr uint16_t default_port = 8800;
  uint16_t port;
  if (argc > 1)
  {
    port = atoi(argv[1]);
  }
  else
  {
    port = default_port;
  }

  RunServer(port);
  CleanupTracer();
  return 0;
}

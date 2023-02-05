// Make sure to include GRPC headers first because otherwise Abseil may create
// ambiguity with `nostd::variant` if compiled with Visual Studio 2015. Other
// modern compilers are unaffected.
#include <grpcpp/grpcpp.h>
#ifdef BAZEL_BUILD
#  include "examples/grpc/protos/messages.grpc.pb.h"
#else
#  include "messages.grpc.pb.h"
#endif

#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <chrono>
#include <unordered_map>

#include "opentelemetry/trace/semantic_conventions.h"
#include "opentelemetry/nostd/string_view.h"
#include "tracer_common.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;

using grpc_example::Greeter;
using grpc_example::GreetRequest;
using grpc_example::GreetResponse;

namespace
{
namespace context = opentelemetry::context;
using namespace opentelemetry::trace;
using RpcHeaderT = std::map<std::string, std::string>;
static const std::string libVersion = "zipkin_grpc_client_lib";

class GreeterClient
{
public:
  GreeterClient(std::shared_ptr<Channel> channel) : stub_(Greeter::NewStub(channel)) {}

  std::string Greet(std::string ip, uint16_t port)
  {
    // Build gRPC Context objects and protobuf message containers
    GreetRequest request;
    GreetResponse response;
    ClientContext context;

    request.set_m_message("This is a message from greeter client.");

    StartSpanOptions options;
    options.kind = SpanKind::kClient;

    std::string span_name = "GreeterClient/Greet";
    auto span             = get_tracer(libVersion)->StartSpan(
        span_name,
        {{SemanticConventions::kRpcSystem, "grpc"},
         {SemanticConventions::kRpcService, "grpc-example.GreetService"},
         {SemanticConventions::kRpcMethod, "Greet"},
         {SemanticConventions::kNetSockPeerAddr, ip},
         {SemanticConventions::kNetPeerPort, port}},
        options);
    // span->SetAttribute();
    auto scope = get_tracer(libVersion)->WithActiveSpan(span);

    // inject current context to grpc metadata
    auto current_ctx = context::RuntimeContext::GetCurrent();
    RpcHeaderT attributes;
    BondRpcTextMapCarrier<RpcHeaderT> carrier {&attributes};
    auto prop = context::propagation::GlobalTextMapPropagator::GetGlobalPropagator();
    prop->Inject(carrier, current_ctx);

    for(const auto& pair : *carrier.m_headers)
    {
        (*request.mutable_m_spancontext())[pair.first] = pair.second;
    }

    // Send request to server
    Status status = stub_->Greet(&context, request, &response);
    if (status.ok())
    {
      span->SetStatus(StatusCode::kOk);
      span->SetAttribute(SemanticConventions::kRpcGrpcStatusCode, status.error_code());
      // Make sure to end your spans!
      span->End();
      return response.m_message();
    }
    else
    {
      std::cout << status.error_code() << ": " << status.error_message() << std::endl;
      span->SetStatus(StatusCode::kError);
      span->SetAttribute(SemanticConventions::kRpcGrpcStatusCode, status.error_code());
      // Make sure to end your spans!
      span->End();
      return "RPC failed";
    }
  }

private:
  std::unique_ptr<Greeter::Stub> stub_;
};  // GreeterClient class

void RunClient(uint16_t port)
{
  GreeterClient greeter(
      grpc::CreateChannel("0.0.0.0:" + std::to_string(port), grpc::InsecureChannelCredentials()));
  std::string response = greeter.Greet("0.0.0.0", port);
  std::cout << "grpc_server says: " << response << std::endl;
}
}  // namespace

int main(int argc, char **argv)
{
  InitClientTracer();
  // set global propagator
  context::propagation::GlobalTextMapPropagator::SetGlobalPropagator(
      opentelemetry::nostd::shared_ptr<context::propagation::TextMapPropagator>(
          new propagation::HttpTraceContext()));
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

  std::string rootSpanName = "rootSpan";
  StartSpanOptions opts;
  opts.kind = SpanKind::kClient;
  auto rootSpan = get_tracer(libVersion)->StartSpan(rootSpanName, {}, opts);
  auto scope = get_tracer(libVersion)->WithActiveSpan(rootSpan);

  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  RunClient(port);

  rootSpan->End();

  CleanupTracer();
  
  return 0;
}

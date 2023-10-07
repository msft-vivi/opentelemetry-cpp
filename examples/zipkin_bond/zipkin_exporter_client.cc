#include <iostream>
#include <string>
#include <chrono>
#include <random>
#include "opentelemetry/ext/http/client/http_client_factory.h"
#include "nlohmann/json.hpp"
#include "opentelemetry/exporters/zipkin/zipkin_exporter_options.h"

namespace http_client = opentelemetry::ext::http::client;
namespace exporter = opentelemetry::exporter::zipkin;

// sizeof count the null terminator, the result of sizeof(charMap) is 37
// const char charMap[] = "0123456789abcdefghijklmnopqrstuvwxyz";
const char charMap[] = "0123456789abcde";

std::string NewGuid()
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(0, sizeof(charMap) - 2); // distribution in range
    char res[32];

    for (size_t i = 0; i < 32; ++i)
    {
        auto idx = dist(rng);
        assert(idx >= 0 && idx < sizeof(charMap));
        res[i] = charMap[idx];
    }

    return std::string(res, 32);
}

int main()
{
    auto http_client = opentelemetry::ext::http::client::HttpClientFactory::CreateSync();
    auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch());
    exporter::ZipkinExporterOptions options;

    auto traceID = NewGuid();
    std::cout << "traceID: " << traceID << std::endl;

    nlohmann::json span;
    span["duration"] = 2000924;
    span["id"] = "271116278f9d5446";
    span["kind"] = "CLIENT";
    span["name"] = "ClientSpan";
    span["timestamp"] = timestamp.count() - 100000;
    span["traceId"] = traceID;
    span["localEndpoint"]["serviceName"] = "ClientLib";
    span["tags"]["attrA"] = "valueA";
    span["tags"]["attrB"] = "valueB";

    nlohmann::json span2;
    span2["duration"] = 3000924;
    span2["id"] = "271116278f9d5336";
    span2["kind"] = "SERVER";
    span2["name"] = "ServerSpan1";
    span2["parentId"] = "271116278f9d5446";
    span2["timestamp"] = timestamp.count() - 50000;
    span2["traceId"] = traceID;
    span2["localEndpoint"]["serviceName"] = "ServiceLib1";

    nlohmann::json span3;
    span3["duration"] = 2000924;
    span3["id"] = "271116278f9d5226";
    span3["kind"] = "SERVER";
    span3["name"] = "ServerSpan1";
    span3["parentId"] = "271116278f9d5446";
    span3["timestamp"] = timestamp.count();
    span3["traceId"] = traceID;
    span3["localEndpoint"]["serviceName"] = "ServiceLib2";

    nlohmann::json jsonSpans = { span, span2, span3 };
    // jsonSpans.push_back(span);

    // std::string bodyString = R"([{"duration":2016924,"id":"271116278f9d5446","kind":"CLIENT","localEndpoint":{"port":9411,"serviceName":"my-test-client"},"name":"rootSpan","tags":{"otel.library.name":"zipkin_grpc_client_lib","otel.library.version":""},"timestamp":1696645649100016,"traceId":"cf8dddf53604a9c55090ae714a34b146"},
    // {"duration":306343,"id":"e805d7bfe5f48e5e","kind":"CLIENT","localEndpoint":{"port":9411,"serviceName":"rpc-client"},"name":"ClientSpan","parentId":"271116278f9d5446","tags":{"attrA":"valueA","attrB":"valueB","net.peer.port":8800,"net.sock.peer.addr":"0.0.0.0","otel.library.name":"zipkin_grpc_client_lib","otel.library.version":"","otel.status_code":1,"rpc.grpc.status_code":0,"rpc.method":"Greet","rpc.service":"grpc-example.GreetService","rpc.system":"grpc"},"timestamp":1696645649100018,"traceId":"cf8dddf53604a9c55090ae714a34b146"}])";
    // std::string bodyString = R"([{"duration":500158,"id":"6a977443ce630fc9","kind":"SERVER","localEndpoint":{"port":9411,"serviceName":"bondrpc-server"},"name":"ServerSpan","parentId":"e805d7bfe5f48e5e","tags":{"otel.library.name":"grpc_server_lib","otel.library.version":"","otel.status_code":1,"rpc.grpc.status_code":0,"rpc.method":"Greet","rpc.service":"GreeterService","rpc.system":"grpc"},"timestamp":1696407239758419,"traceId":"cf8dddf53604a9c55090ae714a34b146"}])";
    // std::string bodyString = "[{\"duration\":500158,\"id\":\"6a977443ce630fc9\",\"kind\":\"SERVER\",\"localEndpoint\":{\"port\":9411,\"serviceName\":\"bondrpc-server\"},\"name\":\"ServerSpan\",\"parentId\":\"e805d7bfe5f48e5e\",\"tags\":{\"otel.library.name\":\"grpc_server_lib\",\"otel.library.version\":\"\",\"otel.status_code\":1,\"rpc.grpc.status_code\":0,\"rpc.method\":\"Greet\",\"rpc.service\":\"GreeterService\",\"rpc.system\":\"grpc\"},\"timestamp\":1696407239758419,\"traceId\":\"cf8dddf53604a9c55090ae714a34b146\"}]";
    auto dumpSpan = jsonSpans.dump();
    std::cout << "dump spans: " << dumpSpan << std::endl;

    http_client::Body body_v(dumpSpan.begin(), dumpSpan.end());
    opentelemetry::ext::http::common::UrlParser url_parser(options.endpoint);
    auto result = http_client->Post(url_parser.url_, body_v, options.headers);
    if (result && (result.GetResponse().GetStatusCode() == 200 || result.GetResponse().GetStatusCode() == 202))
    {
      std::cout << "Post to zipkin http server successfully" << std::endl;
    }
    else
    {
      if (result.GetSessionState() == http_client::SessionState::ConnectFailed)
      {
        std::cout << "ZIPKIN EXPORTER] Zipkin Exporter: Connection failed" << std::endl;
      }
      else
      {
        std::cout << "ZIPKIN EXPORTER] Zipkin Exporter faild: unknown error" << std::endl;
      }
    }

    return 0;
}
// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "opentelemetry/context/propagation/global_propagator.h"
#include "opentelemetry/context/propagation/text_map_propagator.h"
#include "opentelemetry/exporters/zipkin/zipkin_exporter_factory.h"
#include "opentelemetry/nostd/shared_ptr.h"
#include "opentelemetry/sdk/trace/simple_processor_factory.h"
#include "opentelemetry/sdk/trace/tracer_context_factory.h"
#include "opentelemetry/sdk/trace/tracer_provider_factory.h"
#include "opentelemetry/trace/propagation/http_trace_context.h"
#include "opentelemetry/trace/provider.h"

#include <grpcpp/grpcpp.h>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

// #define ENABLE_SLEEP

using grpc::ClientContext;
using grpc::ServerContext;

namespace resource = opentelemetry::sdk::resource;

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

template <typename T>
class BondRpcTextMapCarrier : public opentelemetry::context::propagation::TextMapCarrier
{
public:
  BondRpcTextMapCarrier<T>(T *p_headers) : m_headers(p_headers) {}
  BondRpcTextMapCarrier() = default;
  virtual opentelemetry::nostd::string_view Get(
      opentelemetry::nostd::string_view key) const noexcept override
  {
    auto iter = m_headers->find(key.data());
    if (iter != m_headers->end())
    {
      return iter->second;
    }
    return "";
  }

  virtual void Set(opentelemetry::nostd::string_view key,
                   opentelemetry::nostd::string_view value) noexcept override
  {
    // m_headers->insert(std::make_pair(key.data(), value.data()));
    m_headers->insert(std::make_pair(std::string(key), std::string(value)));
  }

  // Public visibility member
  T *m_headers;
};

void InitQueryMasterTracer()
{
  opentelemetry::exporter::zipkin::ZipkinExporterOptions opts;

  resource::ResourceAttributes attributes = {{"service.name", "QueryMasterService"}};
  auto resources = resource::Resource::Create(attributes);

  auto exporter  = opentelemetry::exporter::zipkin::ZipkinExporterFactory::Create(opts);
  auto processor =
      opentelemetry::sdk::trace::SimpleSpanProcessorFactory::Create(std::move(exporter));

  std::vector<std::unique_ptr<opentelemetry::sdk::trace::SpanProcessor>> processors;
  processors.push_back(std::move(processor));

  // Default is an always-on sampler.
  std::shared_ptr<opentelemetry::sdk::trace::TracerContext> context =
      opentelemetry::sdk::trace::TracerContextFactory::Create(std::move(processors), resources);

  std::shared_ptr<opentelemetry::trace::TracerProvider> provider =
      opentelemetry::sdk::trace::TracerProviderFactory::Create(context);

  // Set the global trace provider
  opentelemetry::trace::Provider::SetTracerProvider(provider);

  // set global propagator
  opentelemetry::context::propagation::GlobalTextMapPropagator::SetGlobalPropagator(
      opentelemetry::nostd::shared_ptr<opentelemetry::context::propagation::TextMapPropagator>(
          new opentelemetry::trace::propagation::HttpTraceContext()));
}

void InitSaaSTracer()
{
  opentelemetry::exporter::zipkin::ZipkinExporterOptions opts;

  resource::ResourceAttributes attributes = {{"service.name", "SaaSService"}};
  auto resources = resource::Resource::Create(attributes);

  auto exporter  = opentelemetry::exporter::zipkin::ZipkinExporterFactory::Create(opts);
  auto processor =
      opentelemetry::sdk::trace::SimpleSpanProcessorFactory::Create(std::move(exporter));

  std::vector<std::unique_ptr<opentelemetry::sdk::trace::SpanProcessor>> processors;
  processors.push_back(std::move(processor));

  // Default is an always-on sampler.
  std::shared_ptr<opentelemetry::sdk::trace::TracerContext> context =
      opentelemetry::sdk::trace::TracerContextFactory::Create(std::move(processors), resources);

  std::shared_ptr<opentelemetry::trace::TracerProvider> provider =
      opentelemetry::sdk::trace::TracerProviderFactory::Create(context);

  // Set the global trace provider
  opentelemetry::trace::Provider::SetTracerProvider(provider);

  // set global propagator
  opentelemetry::context::propagation::GlobalTextMapPropagator::SetGlobalPropagator(
      opentelemetry::nostd::shared_ptr<opentelemetry::context::propagation::TextMapPropagator>(
          new opentelemetry::trace::propagation::HttpTraceContext()));
}

void InitRaaSTracer()
{
  opentelemetry::exporter::zipkin::ZipkinExporterOptions opts;

  resource::ResourceAttributes attributes = {{"service.name", "RaaSService"}};
  auto resources = resource::Resource::Create(attributes);

  auto exporter  = opentelemetry::exporter::zipkin::ZipkinExporterFactory::Create(opts);
  auto processor =
      opentelemetry::sdk::trace::SimpleSpanProcessorFactory::Create(std::move(exporter));

  std::vector<std::unique_ptr<opentelemetry::sdk::trace::SpanProcessor>> processors;
  processors.push_back(std::move(processor));

  // Default is an always-on sampler.
  std::shared_ptr<opentelemetry::sdk::trace::TracerContext> context =
      opentelemetry::sdk::trace::TracerContextFactory::Create(std::move(processors), resources);

  std::shared_ptr<opentelemetry::trace::TracerProvider> provider =
      opentelemetry::sdk::trace::TracerProviderFactory::Create(context);

  // Set the global trace provider
  opentelemetry::trace::Provider::SetTracerProvider(provider);

  // set global propagator
  opentelemetry::context::propagation::GlobalTextMapPropagator::SetGlobalPropagator(
      opentelemetry::nostd::shared_ptr<opentelemetry::context::propagation::TextMapPropagator>(
          new opentelemetry::trace::propagation::HttpTraceContext()));
}

void InitSraTracer()
{
  opentelemetry::exporter::zipkin::ZipkinExporterOptions opts;

  resource::ResourceAttributes attributes = {{"service.name", "SraService"}};
  auto resources = resource::Resource::Create(attributes);

  auto exporter  = opentelemetry::exporter::zipkin::ZipkinExporterFactory::Create(opts);
  auto processor =
      opentelemetry::sdk::trace::SimpleSpanProcessorFactory::Create(std::move(exporter));

  std::vector<std::unique_ptr<opentelemetry::sdk::trace::SpanProcessor>> processors;
  processors.push_back(std::move(processor));

  // Default is an always-on sampler.
  std::shared_ptr<opentelemetry::sdk::trace::TracerContext> context =
      opentelemetry::sdk::trace::TracerContextFactory::Create(std::move(processors), resources);

  std::shared_ptr<opentelemetry::trace::TracerProvider> provider =
      opentelemetry::sdk::trace::TracerProviderFactory::Create(context);

  // Set the global trace provider
  opentelemetry::trace::Provider::SetTracerProvider(provider);

  // set global propagator
  opentelemetry::context::propagation::GlobalTextMapPropagator::SetGlobalPropagator(
      opentelemetry::nostd::shared_ptr<opentelemetry::context::propagation::TextMapPropagator>(
          new opentelemetry::trace::propagation::HttpTraceContext()));
}

void CleanupTracer()
{
  std::shared_ptr<opentelemetry::trace::TracerProvider> none;
  opentelemetry::trace::Provider::SetTracerProvider(none);
}

opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> get_tracer(std::string p_tracerName,
                                                                          std::string p_version = "")
{
  auto provider = opentelemetry::trace::Provider::GetTracerProvider();
  if (!p_version.empty())
  {
    return provider->GetTracer(p_tracerName, p_version);
  }

  return provider->GetTracer(p_tracerName);
}

}  // namespace

# Proto file
get_filename_component(proto_file "./protos/messages.proto" ABSOLUTE)
get_filename_component(proto_file_path "${proto_file}" PATH)

message("PATH:${proto_file_path}:${proto_file}")
# Generated sources
set(zipkin_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/messages.pb.cc")
set(zipkin_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/messages.pb.h")
set(zipkin_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/messages.grpc.pb.cc")
set(zipkin_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/messages.grpc.pb.h")

add_custom_command(
  OUTPUT "${zipkin_proto_srcs}" "${zipkin_proto_hdrs}" "${zipkin_grpc_srcs}"
         "${zipkin_grpc_hdrs}"
  COMMAND
    ${PROTOBUF_PROTOC_EXECUTABLE} ARGS "--grpc_out=${CMAKE_CURRENT_BINARY_DIR}"
    "--cpp_out=${CMAKE_CURRENT_BINARY_DIR}" "--proto_path=${proto_file_path}"
    "--plugin=protoc-gen-grpc=${gRPC_CPP_PLUGIN_EXECUTABLE}" "${proto_file}")

add_library(zipkin_bond_proto ${zipkin_grpc_srcs} ${zipkin_grpc_hdrs}
                               ${zipkin_proto_srcs} ${zipkin_proto_hdrs})

include(${PROJECT_SOURCE_DIR}/cmake/proto-options-patch.cmake)
patch_protobuf_targets(zipkin_bond_proto)

include_directories(
  ${CMAKE_SOURCE_DIR}/exporters/ostream/include ${CMAKE_SOURCE_DIR}/ext/include
  ${CMAKE_SOURCE_DIR}/api/include/ ${CMAKE_SOURCE_DIR/} ${CMAKE_SOURCE_DIR}/exporters/zipkin/include)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

if(TARGET protobuf::libprotobuf)
  target_link_libraries(zipkin_bond_proto gRPC::grpc++ protobuf::libprotobuf)
else()
  target_include_directories(zipkin_bond_proto ${Protobuf_INCLUDE_DIRS})
  target_link_libraries(zipkin_bond_proto ${Protobuf_LIBRARIES})
endif()

foreach(_target zipkin_bond_client zipkin_bond_server zipkin_exporter_client)
  add_executable(${_target} "${_target}.cc")
  target_link_libraries(
    ${_target} zipkin_bond_proto protobuf::libprotobuf gRPC::grpc++
    opentelemetry_trace opentelemetry_exporter_zipkin_trace)
  patch_protobuf_targets(${_target})
endforeach()

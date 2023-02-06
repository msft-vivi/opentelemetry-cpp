# Proto file
get_filename_component(proto_file "./protos/messages.proto" ABSOLUTE)
get_filename_component(proto_file_path "${proto_file}" PATH)

message("PATH:${proto_file_path}:${proto_file}")

# Generated sources
set(sra_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/messages.pb.cc")
set(sra_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/messages.pb.h")
set(sra_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/messages.grpc.pb.cc")
set(sra_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/messages.grpc.pb.h")

add_custom_command(
    OUTPUT "${sra_proto_srcs}" "${sra_proto_hdrs}" "${sra_grpc_srcs}"
    "${sra_grpc_hdrs}"
    COMMAND
    ${PROTOBUF_PROTOC_EXECUTABLE} ARGS "--grpc_out=${CMAKE_CURRENT_BINARY_DIR}"
    "--cpp_out=${CMAKE_CURRENT_BINARY_DIR}" "--proto_path=${proto_file_path}"
    "--plugin=protoc-gen-grpc=${gRPC_CPP_PLUGIN_EXECUTABLE}" "${proto_file}")

add_library(sra_proto ${sra_grpc_srcs} ${sra_grpc_hdrs}
    ${sra_proto_srcs} ${sra_proto_hdrs})

include(${PROJECT_SOURCE_DIR}/cmake/proto-options-patch.cmake)
patch_protobuf_targets(sra_proto)

include_directories(
    ${CMAKE_SOURCE_DIR}/ext/include
    ${CMAKE_SOURCE_DIR}/api/include/
    ${CMAKE_SOURCE_DIR/}
    ${CMAKE_SOURCE_DIR}/exporters/zipkin/include)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

if(TARGET protobuf::libprotobuf)
    target_link_libraries(sra_proto gRPC::grpc++ protobuf::libprotobuf)
else()
    target_include_directories(sra_proto ${Protobuf_INCLUDE_DIRS})
    target_link_libraries(sra_proto ${Protobuf_LIBRARIES})
endif()

foreach(_target sra raas saas qm)
    add_executable(${_target} "${_target}.cc")
    target_link_libraries(
        ${_target} sra_proto
        protobuf::libprotobuf
        gRPC::grpc++
        opentelemetry_trace
        opentelemetry_exporter_zipkin_trace)
    patch_protobuf_targets(${_target})
endforeach()
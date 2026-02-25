cmake_minimum_required(VERSION 3.20)
project(mcap_reader CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── 查找 Protobuf ──
find_package(Protobuf REQUIRED)
message(STATUS "Protobuf version : ${Protobuf_VERSION}")
message(STATUS "Protobuf protoc  : ${Protobuf_PROTOC_EXECUTABLE}")

# ── 通过 FetchContent 引入 mcap_builder ──
include(FetchContent)
FetchContent_Declare(
  mcap_builder
  GIT_REPOSITORY https://github.com/olympus-robotics/mcap_builder.git
  GIT_TAG        main
)
FetchContent_MakeAvailable(mcap_builder)

# ── 从 student.proto 生成 C++ 源码 ──
set(PROTO_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/../proto)
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

add_custom_command(
  OUTPUT  ${GENERATED_DIR}/student.pb.h ${GENERATED_DIR}/student.pb.cc
  COMMAND ${Protobuf_PROTOC_EXECUTABLE}
          --proto_path=${PROTO_DIR}
          --cpp_out=${GENERATED_DIR}
          ${PROTO_DIR}/student.proto
  DEPENDS ${PROTO_DIR}/student.proto
  COMMENT "Generating protobuf C++ sources for student.proto (reader)"
)

# ── 构建 reader 可执行文件 ──
add_executable(mcap_reader
  main.cpp
  ${GENERATED_DIR}/student.pb.cc
)
target_include_directories(mcap_reader PRIVATE ${GENERATED_DIR})
target_link_libraries(mcap_reader PRIVATE mcap protobuf::libprotobuf)

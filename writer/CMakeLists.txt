cmake_minimum_required(VERSION 3.20)
project(mcap_writer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── 查找 Protobuf ──
find_package(Protobuf REQUIRED)
message(STATUS "Protobuf version : ${Protobuf_VERSION}")
message(STATUS "Protobuf protoc  : ${Protobuf_PROTOC_EXECUTABLE}")

# ── 让 find_package(lz4) 能找到 conda-forge 的 lz4-c ──
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)

# ── 通过 FetchContent 引入 mcap_builder ──
include(FetchContent)
FetchContent_Declare(
  mcap_builder
  GIT_REPOSITORY https://github.com/olympus-robotics/mcap_builder.git
  GIT_TAG        main
)
FetchContent_MakeAvailable(mcap_builder)

# ── 拉取 foxglove-sdk 获取官方 proto 定义（仅下载，不构建） ──
FetchContent_Declare(
  foxglove_schemas
  GIT_REPOSITORY https://github.com/foxglove/foxglove-sdk.git
  GIT_TAG        main
  GIT_SHALLOW    TRUE
  EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(foxglove_schemas)

# ── 从 proto 文件生成 C++ 源码 ──
# 因为 imu.proto 中 import "foxglove/Vector3.proto"，生成的 imu.pb.h
# 会 #include "foxglove/Vector3.pb.h"，所以必须用 add_custom_command
# 保持 foxglove/ 子目录结构（protobuf_generate_cpp 会把文件打平）。
set(LOCAL_PROTO_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/../proto)
set(FOXGLOVE_PROTO_DIR ${foxglove_schemas_SOURCE_DIR}/schemas/proto)
set(PB_OUT ${CMAKE_CURRENT_BINARY_DIR})

add_custom_command(
  OUTPUT
    ${PB_OUT}/student.pb.cc          ${PB_OUT}/student.pb.h
    ${PB_OUT}/imu.pb.cc              ${PB_OUT}/imu.pb.h
    ${PB_OUT}/foxglove/Point3.pb.cc  ${PB_OUT}/foxglove/Point3.pb.h
    ${PB_OUT}/foxglove/Vector3.pb.cc ${PB_OUT}/foxglove/Vector3.pb.h
  COMMAND ${Protobuf_PROTOC_EXECUTABLE}
    --proto_path=${LOCAL_PROTO_DIR}
    --proto_path=${FOXGLOVE_PROTO_DIR}
    --proto_path=${Protobuf_INCLUDE_DIRS}
    --cpp_out=${PB_OUT}
    student.proto  imu.proto
    foxglove/Point3.proto  foxglove/Vector3.proto
  DEPENDS
    ${LOCAL_PROTO_DIR}/student.proto
    ${LOCAL_PROTO_DIR}/imu.proto
    ${FOXGLOVE_PROTO_DIR}/foxglove/Point3.proto
    ${FOXGLOVE_PROTO_DIR}/foxglove/Vector3.proto
  COMMENT "Generating protobuf C++ sources (writer)"
)

set(PROTO_SRCS
  ${PB_OUT}/student.pb.cc
  ${PB_OUT}/imu.pb.cc
  ${PB_OUT}/foxglove/Point3.pb.cc
  ${PB_OUT}/foxglove/Vector3.pb.cc
)

# ── 构建 writer 可执行文件 ──
add_executable(mcap_writer main.cpp ${PROTO_SRCS})
target_include_directories(mcap_writer PRIVATE ${PB_OUT})
target_link_libraries(mcap_writer PRIVATE mcap protobuf::libprotobuf)

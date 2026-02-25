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
protobuf_generate_cpp(STUDENT_SRCS STUDENT_HDRS
  ${CMAKE_CURRENT_SOURCE_DIR}/../proto/student.proto)
protobuf_generate_cpp(POINT3_SRCS POINT3_HDRS
  ${foxglove_schemas_SOURCE_DIR}/schemas/proto/foxglove/Point3.proto)

# ── 构建 writer 可执行文件 ──
add_executable(mcap_writer main.cpp ${STUDENT_SRCS} ${POINT3_SRCS})
target_include_directories(mcap_writer PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(mcap_writer PRIVATE mcap protobuf::libprotobuf)

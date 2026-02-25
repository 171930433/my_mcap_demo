cmake_minimum_required(VERSION 3.20)
project(mcap_reader CXX)

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

# ── 从 student.proto 生成 C++ 源码 ──
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${CMAKE_CURRENT_SOURCE_DIR}/../proto/student.proto)

# ── 构建 reader 可执行文件 ──
add_executable(mcap_reader main.cpp ${PROTO_SRCS})
target_include_directories(mcap_reader PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(mcap_reader PRIVATE mcap protobuf::libprotobuf)

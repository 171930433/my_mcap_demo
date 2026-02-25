// mcap_writer: 将 10 条 Student 和 10 条 foxglove Point3 protobuf 消息写入 mcap 文件

#include <mcap/writer.hpp>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>

#include "student.pb.h"
#include "Point3.pb.h"

#include <chrono>
#include <cmath>
#include <iostream>
#include <queue>
#include <string>
#include <unordered_set>

// ---------------------------------------------------------------------------
// 构建 FileDescriptorSet（包含目标 descriptor 及其所有传递依赖）。
// MCAP 用序列化后的 FileDescriptorSet 作为 protobuf schema 的数据。
// ---------------------------------------------------------------------------
google::protobuf::FileDescriptorSet BuildFileDescriptorSet(
    const google::protobuf::Descriptor* toplevel) {
  google::protobuf::FileDescriptorSet fdSet;
  std::queue<const google::protobuf::FileDescriptor*> toAdd;
  toAdd.push(toplevel->file());
  std::unordered_set<std::string> seen;
  while (!toAdd.empty()) {
    const google::protobuf::FileDescriptor* fd = toAdd.front();
    toAdd.pop();
    fd->CopyTo(fdSet.add_file());
    for (int i = 0; i < fd->dependency_count(); ++i) {
      const auto* dep = fd->dependency(i);
      if (seen.find(dep->name()) == seen.end()) {
        seen.insert(dep->name());
        toAdd.push(dep);
      }
    }
  }
  return fdSet;
}

// 返回当前时间的纳秒时间戳
mcap::Timestamp now() {
  return mcap::Timestamp(
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::system_clock::now().time_since_epoch())
          .count());
}

int main() {
  const std::string outputFile = "students.mcap";

  // ── 打开 MCAP 写入器 ──
  mcap::McapWriter writer;
  auto options = mcap::McapWriterOptions("");
  auto status = writer.open(outputFile, options);
  if (!status.ok()) {
    std::cerr << "Failed to open " << outputFile << ": " << status.message
              << std::endl;
    return 1;
  }

  // ── 注册 protobuf Schema ──
  mcap::Schema schema(
      "demo.Student", "protobuf",
      BuildFileDescriptorSet(demo::Student::descriptor()).SerializeAsString());
  writer.addSchema(schema);

  // ── 注册 Channel ──
  mcap::Channel channel("students", "protobuf", schema.id);
  writer.addChannel(channel);

  // ── 写入 10 条 Student 消息 ──
  const std::string names[] = {"Alice",   "Bob",   "Charlie", "Diana", "Eve",
                               "Frank",   "Grace", "Henry",   "Ivy",   "Jack"};

  for (int i = 0; i < 10; ++i) {
    demo::Student student;
    student.set_id(i + 1);
    student.set_name(names[i]);
    student.set_age(18 + (i % 5));
    student.set_email(names[i] + "@example.com");
    student.set_score(75.0f + static_cast<float>(i) * 2.5f);

    std::string serialized = student.SerializeAsString();

    mcap::Message msg;
    msg.channelId = channel.id;
    msg.sequence = static_cast<uint32_t>(i);
    msg.publishTime = now();
    msg.logTime = msg.publishTime;
    msg.data = reinterpret_cast<const std::byte*>(serialized.data());
    msg.dataSize = serialized.size();

    auto res = writer.write(msg);
    if (!res.ok()) {
      std::cerr << "Failed to write message " << i << ": " << res.message
                << std::endl;
      writer.terminate();
      return 1;
    }

    std::cout << student.DebugString() << std::endl;
  }

  // ── 注册 Point3 Schema（来自 foxglove-sdk） ──
  mcap::Schema point3Schema(
      "foxglove.Point3", "protobuf",
      BuildFileDescriptorSet(foxglove::Point3::descriptor())
          .SerializeAsString());
  writer.addSchema(point3Schema);

  // ── 注册 Point3 Channel ──
  mcap::Channel point3Channel("points", "protobuf", point3Schema.id);
  writer.addChannel(point3Channel);

  // ── 写入 10 条 Point3 消息 ──
  std::cout << "\n--- Writing Point3 messages ---\n" << std::endl;
  for (int i = 0; i < 10; ++i) {
    foxglove::Point3 pt;
    pt.set_x(static_cast<double>(i));
    pt.set_y(std::sin(static_cast<double>(i)));
    pt.set_z(static_cast<double>(i) * 0.5);

    std::string serialized = pt.SerializeAsString();

    mcap::Message msg;
    msg.channelId = point3Channel.id;
    msg.sequence = static_cast<uint32_t>(i);
    msg.publishTime = now();
    msg.logTime = msg.publishTime;
    msg.data = reinterpret_cast<const std::byte*>(serialized.data());
    msg.dataSize = serialized.size();

    auto res = writer.write(msg);
    if (!res.ok()) {
      std::cerr << "Failed to write Point3 message " << i << ": "
                << res.message << std::endl;
      writer.terminate();
      return 1;
    }

    std::cout << pt.DebugString() << std::endl;
  }

  writer.close();
  std::cout << "\nSuccessfully wrote 10 Student + 10 Point3 messages to "
            << outputFile << std::endl;

  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}

// mcap_reader: 从 mcap 文件中读取 Student 和 foxglove Point3 消息并打印

#include <mcap/reader.hpp>

#include "student.pb.h"
#include "Point3.pb.h"

#include <iostream>
#include <string>

int main() {
  const std::string inputFile = "students.mcap";

  // ── 打开 MCAP 读取器 ──
  mcap::McapReader reader;
  auto status = reader.open(inputFile);
  if (!status.ok()) {
    std::cerr << "Failed to open " << inputFile << ": " << status.message
              << std::endl;
    return 1;
  }

  std::cout << "Reading messages from " << inputFile << " :\n" << std::endl;

  // ── 遍历所有消息，按 schema 名称分别解析 ──
  int studentCount = 0;
  int point3Count = 0;
  auto messageView = reader.readMessages();
  for (auto it = messageView.begin(); it != messageView.end(); ++it) {
    const auto& schemaName = it->schema->name;

    if (schemaName == "demo.Student") {
      demo::Student student;
      if (!student.ParseFromArray(it->message.data,
                                  static_cast<int>(it->message.dataSize))) {
        std::cerr << "Failed to parse Student message" << std::endl;
        continue;
      }
      std::cout << "[Student] " << student.DebugString() << std::endl;
      ++studentCount;
    } else if (schemaName == "foxglove.Point3") {
      foxglove::Point3 pt;
      if (!pt.ParseFromArray(it->message.data,
                             static_cast<int>(it->message.dataSize))) {
        std::cerr << "Failed to parse Point3 message" << std::endl;
        continue;
      }
      std::cout << "[Point3] " << pt.DebugString() << std::endl;
      ++point3Count;
    } else {
      std::cerr << "Unknown schema: " << schemaName << std::endl;
    }
  }

  std::cout << "Total: " << studentCount << " Student + " << point3Count
            << " Point3 messages read." << std::endl;

  reader.close();
  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}

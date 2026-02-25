// mcap_reader: 从 mcap 文件中读取 Student protobuf 消息并打印

#include <mcap/reader.hpp>

#include "student.pb.h"

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

  std::cout << "Reading student messages from " << inputFile << " :\n"
            << std::endl;

  // ── 遍历所有消息 ──
  int count = 0;
  auto messageView = reader.readMessages();
  for (auto it = messageView.begin(); it != messageView.end(); ++it) {
    // 反序列化 protobuf 消息
    demo::Student student;
    if (!student.ParseFromArray(it->message.data,
                                static_cast<int>(it->message.dataSize))) {
      std::cerr << "Failed to parse message #" << count << std::endl;
      continue;
    }

    std::cout << "Student #" << (count + 1) << ":" << std::endl;
    std::cout << "  ID    : " << student.id() << std::endl;
    std::cout << "  Name  : " << student.name() << std::endl;
    std::cout << "  Age   : " << student.age() << std::endl;
    std::cout << "  Email : " << student.email() << std::endl;
    std::cout << "  Score : " << student.score() << std::endl;
    std::cout << std::endl;

    ++count;
  }

  std::cout << "Total: " << count << " student messages read." << std::endl;

  reader.close();
  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}

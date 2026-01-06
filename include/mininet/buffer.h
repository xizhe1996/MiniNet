#pragma once
#include <sys/types.h>

#include <cstddef>
#include <string>
#include <vector>
namespace mininet {

class Buffer {
 public:
  Buffer();

  // 当前可读字节数
  size_t readable_bytes() const;

  // 当前可写空间（vector 末尾到 capacity）
  size_t writable_bytes() const;

  // 返回可读数据指针（只读）
  const char* peek() const;

  // 追加数据
  void append(const char* data, size_t len);
  void append(const std::string& s);

  // 消费 n 字节
  void retrieve(size_t len);

  // 消费全部
  void retrieve_all();

  // 读出全部数据为 string，并清空
  std::string retrieve_all_as_string();

  // wirtable 区指针
  char* begin_write();
  const char* begin_write() const;

  // 推进write_index_
  void has_written(size_t len);

  // 从fd阅读到buffer
  ssize_t read_fd(int fd, int* saved_errno);

 private:
  void ensure_writable_bytes(size_t len);
  void make_space(size_t len);

 private:
  std::vector<char> buffer_;
  size_t read_idx_;
  size_t write_idx_;
};

}  // namespace mininet

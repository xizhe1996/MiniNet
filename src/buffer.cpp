#include "mininet/buffer.h"

#include <algorithm>
#include <cstring>
namespace mininet {

Buffer::Buffer() : buffer_(1024), read_idx_(0), write_idx_(0) {}

size_t Buffer::readable_bytes() const { return write_idx_ - read_idx_; }

size_t Buffer::writable_bytes() const { return buffer_.size() - write_idx_; }

const char* Buffer::peek() const { return buffer_.data() + read_idx_; }

void Buffer::make_space(size_t len) {
  if (read_idx_ + writable_bytes() >= len) {
    size_t readable = readable_bytes();
    std::memmove(buffer_.data(), peek(), readable);
    read_idx_ = 0;
    write_idx_ = readable;
  } else {
    buffer_.resize(write_idx_ + len);
  }
}

void Buffer::ensure_writable_bytes(size_t len) {
  if (len > writable_bytes()) {
    make_space(len);
  }
}

void Buffer::append(const char* data, size_t len) {
  ensure_writable_bytes(len);
  std::copy(data, data + len, buffer_.data() + write_idx_);
  write_idx_ += len;
}

void Buffer::append(const std::string& s) { append(s.data(), s.size()); }

void Buffer::retrieve_all() {
  read_idx_ = 0;
  write_idx_ = 0;
}

void Buffer::retrieve(size_t len) {
  if (len >= readable_bytes()) {
    retrieve_all();
  } else {
    read_idx_ += len;
  }
}

std::string Buffer::retrieve_all_as_string() {
  std::string str(peek(), readable_bytes());
  retrieve_all();
  return str;
}

}  // namespace mininet

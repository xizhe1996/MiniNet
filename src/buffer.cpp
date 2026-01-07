#include "mininet/buffer.h"

#include <errno.h>
#include <sys/uio.h>  // readv

#include <algorithm>
#include <cstring>
namespace mininet {

Buffer::Buffer() : buffer_(1024), read_idx_(0), write_idx_(0) {}

size_t Buffer::readable_bytes() const { return write_idx_ - read_idx_; }

size_t Buffer::writable_bytes() const { return buffer_.size() - write_idx_; }

const char* Buffer::peek() const { return buffer_.data() + read_idx_; }

void Buffer::make_space(size_t len) {
  // If writable is enough, nothing to do.
  if (writable_bytes() >= len) return;

  // Otherwise, try to compact or resize.
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

char* Buffer::begin_write() { return buffer_.data() + write_idx_; }

const char* Buffer::begin_write() const { return buffer_.data() + write_idx_; }

void Buffer::has_written(size_t len) { write_idx_ += len; }

ssize_t Buffer::read_fd(int fd, int* saved_errno) {
  // 常见tcp缓冲区大小 64kb
  char extrabuf[65536];

  iovec io[2];
  const size_t writable = writable_bytes();
  io[0].iov_base = begin_write();
  io[0].iov_len = writable;
  io[1].iov_base = extrabuf;
  io[1].iov_len = sizeof(extrabuf);

  const int iolen = (writable < sizeof(extrabuf)) ? 2 : 1;

  ssize_t n = ::readv(fd, io, iolen);

  if (n < 0) {
    // error, save errno
    if (saved_errno) *saved_errno = errno;
  } else if (static_cast<size_t>(n) <= writable) {
    // n <= writable
    has_written(static_cast<size_t>(n));
  } else {
    // n > writable
    has_written(writable);
    append(extrabuf, n - writable);
  }

  return n;
}

ssize_t Buffer::write_fd(int fd, int* saved_errno) {
  // buf --> write fd
  size_t nreadable = readable_bytes();
  ssize_t n = ::write(fd, peek(), nreadable);

  if (n < 0) {
    // save errno
    if (saved_errno) *saved_errno = errno;
  } else {
    retrieve(static_cast<size_t>(n));
  }
  return n;
}

}  // namespace mininet

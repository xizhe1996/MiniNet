#pragma once
#include <unistd.h>

namespace mininet {

class Fd {
 public:
  Fd() = default;
  explicit Fd(int fd) : fd_(fd) {};

  Fd(const Fd&) = delete;
  Fd& operator=(const Fd&) = delete;

  Fd(Fd&& fd) noexcept : fd_(fd.fd_) { fd.fd_ = -1; };
  Fd& operator=(Fd&& fd) {
    if (this != &fd) {
      if (fd_ >= 0) ::close(fd_);
      fd_ = fd.fd_;
      fd.fd_ = -1;
    }
    return *this;
  }

  ~Fd() {
    if (fd_ >= 0) ::close(fd_);
    fd_ = -1;
  }

  int get() const { return fd_; }

  bool valid() const { return (fd_ >= 0); }

  int release() {
    int tmp = fd_;
    fd_ = -1;
    return tmp;
  }

  void reset(int newfd = -1) {
    if (fd_ >= 0) ::close(fd_);
    fd_ = newfd;
  }

 private:
  int fd_ = -1;
};

};  // namespace mininet

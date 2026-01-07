#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>

#include "mininet/buffer.h"

using mininet::Buffer;

int main() {
  int fds[2];
  if (pipe(fds) < 0) {
    std::cerr << "pipe failed\n";
    return 1;
  }
  int rfd = fds[0];
  int wfd = fds[1];

  Buffer buf;
  buf.append("hello ", 6);
  buf.append("mininet day06!", 13);

  int saved_errno = 0;
  ssize_t wn = buf.write_fd(wfd, &saved_errno);

  std::cout << "write_fd wn=" << wn << " errno=" << saved_errno << "\n";
  std::cout << "after write: buffer readable=" << buf.readable_bytes() << "\n";

  ::close(wfd);

  char tmp[1024];
  ssize_t rn = ::read(rfd, tmp, sizeof(tmp));
  ::close(rfd);

  std::cout << "read back rn=" << rn << "\n";
  std::cout << "content=" << std::string(tmp, rn) << "\n";

  return 0;
}

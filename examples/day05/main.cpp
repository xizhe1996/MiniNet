#include <unistd.h>  // pipe, write, close

#include <iostream>
#include <string>

#include "mininet/buffer.h"

using mininet::Buffer;

int main() {
  int fd[2];
  if (pipe(fd) < 0) {
    std::cerr << "fd failed.\n";
    return -1;
  }

  int readfd = fd[0];
  int writefd = fd[1];

  ssize_t n = ::write(writefd, "hello ", 6);
  n = ::write(writefd, "world!", 6);
  ::close(writefd);

  Buffer buf;
  int save_errno = 0;
  n = buf.read_fd(readfd, &save_errno);
  ::close(readfd);

  std::cout << "read_fd: n is " << n << "\n";
  std::cout << "read_fd: save_errno is " << save_errno << "\n";
  std::cout << "buf data is: " << buf.retrieve_all_as_string() << "\n";

  return 0;
}
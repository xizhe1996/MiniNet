#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>

int main() {
  int fd = ::socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    std::cerr << "socket failed\n";
    return 1;
  }

  sockaddr_in addr;
  std::memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(9090);
  ::inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

  if (::connect(fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
    std::cerr << "connect failed\n";
    ::close(fd);
    return 1;
  }

  std::string msg = "hello mininet\n";
  ::write(fd, msg.data(), msg.size());

  char buf[4096];
  ssize_t n = ::read(fd, buf, sizeof(buf));
  if (n > 0) {
    std::cout << "[client] got: " << std::string(buf, buf + n);
  }

  ::close(fd);
  return 0;
}

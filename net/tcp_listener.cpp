#include "tcp_listener.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <cstring>
#include <iostream>

namespace mininet {

using mininet::Fd;
using mininet::TcpListener;

void TcpListener::bind_and_listen(const char* ip, uint16_t port, int backlog) {
  int sock = ::socket(AF_INET, SOCK_STREAM, 0);

  if (sock < 0) throw std::runtime_error("socket fd < 0\n");

  listen_fd_.reset(sock);

  int opt = 1;
  ::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  sockaddr_in addr;
  memset(&addr, 0, sizeof(sockaddr_in));
  addr.sin_port = htons(port);
  addr.sin_family = AF_INET;
  inet_pton(AF_INET, ip, &addr.sin_addr);

  if (::bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0)
    throw std::runtime_error("socket bind failed.\n");

  if (::listen(sock, backlog) < 0)
    throw std::runtime_error("socket listen failed.\n");

  std::cout << "bind_and_listen: ip->" << ip << "port->" << port << std::endl;
}

Fd TcpListener::accept_one() {
  while (true) {
    sockaddr_in peer;
    socklen_t len = sizeof(peer);
    int cfd = ::accept(listen_fd_.get(), (sockaddr*)&peer, &len);
    if (cfd >= 0) {
      return Fd(cfd);
    }

    if (errno == EINTR) {
      continue;
    }

    throw std::runtime_error("accept() failed");
  }
}

int TcpListener::fd() const { return listen_fd_.get(); }

}  // namespace mininet

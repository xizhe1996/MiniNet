#include <unistd.h>

#include <iostream>
#include <string>

#include "net/tcp_listener.h"

int main() {
  mininet::TcpListener listener;

  // TODO: 监听 127.0.0.1:9090，backlog=128
  listener.bind_and_listen("127.0.0.1", 9090, 128);

  while (true) {
    auto conn = listener.accept_one();
    std::cout << "[server] accepted fd=" << conn.get() << "\n";

    // TODO: 最简单实现：单连接阻塞处理（读->原样写回），对端关闭则结束该连接
    // 提示：read 返回 0 表示对端关闭
    char buf[4096];
    while (true) {
      ssize_t n = ::read(conn.get(), buf, sizeof(buf));
      if (n > 0) {
        // TODO: write 全部写完（今天先简单版：直接 write 一次；后面 Day24
        // 再做完整 send_all）
        ssize_t w = ::write(conn.get(), buf, (size_t)n);
        (void)w;
      } else if (n == 0) {
        std::cout << "[server] peer closed\n";
        break;
      } else {
        // TODO: EINTR 重试；其他错误打印并断开
        std::cerr << "[server] read error\n";
        break;
      }
    }
  }

  return 0;
}

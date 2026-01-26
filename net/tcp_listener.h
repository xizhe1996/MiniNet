#include <cstdint>

#include "net/fd.h"

namespace mininet {

class TcpListener {
 public:
  TcpListener() = default;

  void bind_and_listen(const char* ip, uint16_t port, int backlog);

  Fd accept_one();

  int fd() const;

 private:
  Fd listen_fd_;
};

}  // namespace mininet
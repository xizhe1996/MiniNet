#include "mininet/join_thread.h"

namespace mininet {

JoinThread::JoinThread(std::thread&& t) : t_(std::move(t)) {}

JoinThread::~JoinThread() noexcept {
  if (t_.joinable()) t_.join();
}

JoinThread::JoinThread(JoinThread&& other) noexcept : t_(std::move(other.t_)) {}

JoinThread& JoinThread::operator=(JoinThread&& other) noexcept {
  if (this != &other) {
    if (t_.joinable()) t_.join();
    t_ = std::move(other.t_);
  }
  return *this;
}

}  // namespace mininet
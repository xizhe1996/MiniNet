#pragma once
#include <thread>

namespace mininet {

class JoinThread {
 public:
  explicit JoinThread(std::thread&& t);

  ~JoinThread() noexcept;

  JoinThread(const JoinThread&) = delete;
  JoinThread& operator=(const JoinThread&) = delete;

  JoinThread(JoinThread&& other) noexcept;
  JoinThread& operator=(JoinThread&& other) noexcept;

  std::thread& get() { return t_; }
  const std::thread& get() const { return t_; }

 private:
  std::thread t_;
};

}  // namespace mininet

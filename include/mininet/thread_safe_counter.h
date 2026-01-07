#pragma once
#include <cstdint>
#include <mutex>

namespace mininet {

class ThreadSafeCounter {
 public:
  ThreadSafeCounter() : value_(0) {}

  void increment() {
    std::lock_guard<std::mutex> lk(mtx_);
    ++value_;
  };

  std::int64_t get() const {
    std::lock_guard<std::mutex> lk(mtx_);
    return value_;
  }

 private:
  mutable std::mutex mtx_;
  std::int64_t value_;
};

}  // namespace mininet

#pragma once
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <queue>

namespace mininet {

class BlockingQueue {
 public:
  BlockingQueue() = default;

  void push(int x) {
    {
      std::lock_guard<std::mutex> lk(mtx_);
      q_.push(x);
    }
    cv_.notify_one();
  }

  int pop() {
    std::unique_lock<std::mutex> lk(mtx_);
    cv_.wait(lk, [&] { return !q_.empty(); });
    // while(q_.empty()) cv_.wait(lk);

    int value = q_.front();
    q_.pop();
    return value;
  }

  size_t size() const {
    std::lock_guard<std::mutex> lk(mtx_);
    return q_.size();
  }

 private:
  mutable std::mutex mtx_;
  std::condition_variable cv_;
  std::queue<int> q_;
};

}  // namespace mininet

#pragma once
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <queue>

namespace mininet {

template <typename T>
class BlockingQueue {
 public:
  BlockingQueue() : closed_(false) {}

  bool push(const T& x) {
    {
      std::lock_guard<std::mutex> lk(mtx_);
      if (closed_) return false;
      q_.push(x);
    }
    cv_.notify_one();
    return true;
  }

  bool push(T&& x) {
    {
      std::lock_guard<std::mutex> lk(mtx_);
      if (closed_) return false;
      q_.push(std::move(x));
    }
    cv_.notify_one();
    return true;
  }

  template <typename... Args>
  bool emplace(Args&&... args) {
    {
      std::lock_guard<std::mutex> lk(mtx_);
      if (closed_) return false;
      q_.emplace(std::forward<Args>(args)...);
    }
    cv_.notify_one();
    return true;
  }

  bool pop(T& out) {
    std::unique_lock<std::mutex> lk(mtx_);
    cv_.wait(lk, [&] { return !q_.empty() || closed_; });

    if (q_.empty() && closed_) return false;

    out = std::move(q_.front());
    q_.pop();
    return true;
  }

  void close() {
    {
      std::lock_guard<std::mutex> lk(mtx_);
      closed_ = true;
    }
    cv_.notify_all();
  }

  bool closed() const {
    std::lock_guard<std::mutex> lk(mtx_);
    return closed_;
  }

  size_t size() const {
    std::lock_guard<std::mutex> lk(mtx_);
    return q_.size();
  }

 private:
  mutable std::mutex mtx_;
  std::condition_variable cv_;
  std::queue<T> q_;
  bool closed_;
};

}  // namespace mininet

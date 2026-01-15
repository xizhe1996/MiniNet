#pragma once
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <queue>
#include <utility>

namespace mininet {

template <class T>
class BoundedBlockingQueue {
 public:
  explicit BoundedBlockingQueue(size_t capacity)
      : cap_(capacity), closed_(false) {}

  bool push(const T& x) {
    std::unique_lock<std::mutex> lk(mtx_);
    not_full_.wait(lk, [&] { return closed_ || q_.size() < cap_; });
    if (closed_) return false;
    q_.push(x);
    lk.unlock();
    not_empty_.notify_one();
    return true;
  }

  bool push(T&& x) {
    std::unique_lock<std::mutex> lk(mtx_);
    not_full_.wait(lk, [&] { return closed_ || q_.size() < cap_; });
    if (closed_) return false;
    q_.push(std::move(x));
    lk.unlock();
    not_empty_.notify_one();
    return true;
  }

  bool try_push(const T& x) {
    std::unique_lock<std::mutex> lk(mtx_);
    if (closed_) return false;
    if (q_.size() >= cap_) return false;
    q_.push(x);
    lk.unlock();
    not_empty_.notify_one();
    return true;
  }

  bool try_push(T&& x) {
    std::unique_lock<std::mutex> lk(mtx_);
    if (closed_) return false;
    if (q_.size() >= cap_) return false;
    q_.push(std::move(x));
    lk.unlock();
    not_empty_.notify_one();
    return true;
  }

  bool pop(T& out) {
    std::unique_lock<std::mutex> lk(mtx_);
    not_empty_.wait(lk, [&] { return closed_ || !q_.empty(); });
    if (q_.empty() && closed_) return false;
    out = std::move(q_.front());
    q_.pop();
    lk.unlock();
    not_full_.notify_one();
    return true;
  }

  void close() {
    std::lock_guard<std::mutex> lk(mtx_);
    closed_ = true;
    not_empty_.notify_all();
    not_full_.notify_all();
  }

  size_t size() const {
    std::lock_guard<std::mutex> lk(mtx_);
    return q_.size();
  }

 private:
  const size_t cap_;
  mutable std::mutex mtx_;
  std::condition_variable not_full_;
  std::condition_variable not_empty_;
  std::queue<T> q_;
  bool closed_;
};

}  // namespace mininet

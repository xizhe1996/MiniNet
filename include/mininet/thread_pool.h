#pragma once
#include <atomic>
#include <cstddef>
#include <functional>
#include <thread>
#include <vector>

#include "mininet/blocking_queue.h"

namespace mininet {

class ThreadPool {
 public:
  explicit ThreadPool(size_t n) : stopped_(false) {
    workers_.reserve(n);
    for (size_t i = 0; i < n; ++i) {
      workers_.emplace_back([this] { worker_loop(); });
    }
  }

  ~ThreadPool() { stop(); }

  bool submit(std::function<void()> task) {
    if (stopped_.load()) return false;
    return tasks_.push(std::move(task));
  }

  void stop() {
    bool expected = false;
    if (!stopped_.compare_exchange_strong(expected, true)) {
      // already stopped
      return;
    }

    tasks_.close();  // wake all workers

    for (auto& t : workers_) {
      if (t.joinable()) t.join();
    }
  }

 private:
  void worker_loop() {
    std::function<void()> task;
    while (tasks_.pop(task)) {
      task();
    }
    // queue closed and empty -> exit
  }

 private:
  BlockingQueue<std::function<void()>> tasks_;
  std::vector<std::thread> workers_;
  std::atomic<bool> stopped_;
};

}  // namespace mininet

#pragma once
#include <atomic>
#include <cstddef>
#include <functional>
#include <future>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#include "mininet/blocking_queue_v3.h"

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

  template <class F, class... Args>
  auto submit(F&& f, Args&&... args)
      -> std::future<std::invoke_result_t<F, Args...>> {
    using R = std::invoke_result_t<F, Args...>;
    if (stopped_.load()) throw std::runtime_error("threadpool stopped.");

    auto func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
    std::packaged_task<R()> task = std::packaged_task<R()>(std::move(func));
    auto fu = task.get_future();

    auto sp = std::make_shared<std::packaged_task<R()>>(std::move(task));

    if (!tasks_.push([sp] { (*sp)(); }))
      throw std::runtime_error("threadpool closed.");

    return fu;
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
      try {
        task();
      } catch (const std::exception& e) {
        std::cerr << "[worker]" << "task exception : " << e.what() << "\n ";
      } catch (...) {
        std::cerr << "[worker]" << "task unknow exception" << "\n ";
      }
    }
    // queue closed and empty -> exit
  }

 private:
  BlockingQueue<std::function<void()>> tasks_;
  std::vector<std::thread> workers_;
  std::atomic<bool> stopped_;
};

}  // namespace mininet

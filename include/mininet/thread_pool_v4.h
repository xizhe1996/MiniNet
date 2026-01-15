#pragma once
#include <atomic>
#include <cstddef>
#include <functional>
#include <future>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "mininet/bounded_blocking_queue.h"

namespace mininet {

class ThreadPool {
 public:
  explicit ThreadPool(size_t n, size_t capacity)
      : tasks_(capacity), stopped_(false) {
    if (capacity == 0) throw std::invalid_argument("capacity must be > 0");

    workers_.reserve(n);
    for (size_t i = 0; i < n; ++i) {
      workers_.emplace_back([this] { worker_loop(); });
    }
  }

  ~ThreadPool() { stop(); }

  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

  bool try_submit(std::function<void()> task) {
    if (stopped_.load()) return false;
    return tasks_.try_push(std::move(task));
  }

  template <class F, class... Args>
  auto submit(F&& f, Args&&... args)
      -> std::future<std::invoke_result_t<F, Args...>> {
    using R = std::invoke_result_t<F, Args...>;
    if (stopped_.load()) throw std::runtime_error("threadpool stopped.");

    auto taskPtr = std::make_shared<std::packaged_task<R()>>(
        [func = std::forward<F>(f),
         tup = std::make_tuple(std::forward<Args>(args)...)]() mutable -> R {
          if constexpr (std::is_void_v<R>) {
            std::apply(std::move(func), std::move(tup));
            return;
          } else {
            return std::apply(std::move(func), std::move(tup));
          }
        });

    auto fu = taskPtr->get_future();

    if (!tasks_.push([taskPtr] { (*taskPtr)(); }))
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
        // TODO: hook logger later
      } catch (...) {
        // TODO: hook logger later
      }
    }
    // queue closed and empty -> exit
  }

 private:
  BoundedBlockingQueue<std::function<void()>> tasks_;
  std::vector<std::thread> workers_;
  std::atomic<bool> stopped_;
};

}  // namespace mininet

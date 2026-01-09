#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

#include "mininet/thread_pool.h"

using mininet::ThreadPool;

int main() {
  ThreadPool pool(4);

  std::atomic<int> sum{0};

  for (int i = 0; i < 20; ++i) {
    pool.submit([i, &sum] {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      sum.fetch_add(i, std::memory_order_relaxed);
      std::cout << "task " << i << " done\n";
    });
  }

  // 等一会儿让任务执行完（简化版）
  std::this_thread::sleep_for(std::chrono::milliseconds(300));

  pool.stop();
  std::cout << "sum = " << sum.load() << "\n";
  return 0;
}

#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include "mininet/thread_pool_v4.h"

using namespace std::chrono_literals;

int main() {
  mininet::ThreadPool pool(2, 2);

  // case1: future<void>
  {
    auto fut = pool.submit([] { std::this_thread::sleep_for(50ms); });
    fut.get();
    std::cout << "[void] ok\n";
  }

  // case2: move-only 参数（unique_ptr）
  {
    auto fut = pool.submit([](std::unique_ptr<int> p) { return *p + 1; },
                           std::make_unique<int>(41));
    std::cout << "[move-only] " << fut.get() << "\n";  // 42
  }

  // case3: stop 后语义
  pool.stop();
  std::cout << "[after stop] try_submit=" << pool.try_submit([] {}) << "\n";
  try {
    auto fut = pool.submit([] { return 1; });
    std::cout << fut.get() << "\n";
  } catch (const std::exception& e) {
    std::cout << "[after stop] submit throws: " << e.what() << "\n";
  }

  return 0;
}

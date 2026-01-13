#include <iostream>
#include <stdexcept>

#include "mininet/thread_pool.h"

using mininet::ThreadPool;

int main() {
  ThreadPool pool(2);

  // 1) 异常任务
  auto f = pool.submit([]() -> int {
    throw std::runtime_error("boom");
    return 1;
  });

  try {
    std::cout << "result=" << f.get() << "\n";
  } catch (const std::exception& e) {
    std::cout << "caught: " << e.what() << "\n";
  }

  // 2) stop 后 submit
  pool.stop();
  auto f2 = pool.submit([] { return 42; });
  std::cout << "after stop, future valid=" << f2.valid() << "\n";

  return 0;
}

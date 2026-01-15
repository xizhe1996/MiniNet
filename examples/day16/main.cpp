#include <iostream>
#include <stdexcept>

#include "mininet/thread_pool_v3.h"

using mininet::ThreadPool;

int main() {
  ThreadPool pool(2);

  auto f = pool.submit([] { return 123; });
  std::cout << "f=" << f.get() << "\n";

  pool.stop();

  try {
    auto f2 = pool.submit([] { return 456; });
    std::cout << "f2=" << f2.get() << "\n";
  } catch (const std::exception& e) {
    std::cout << "submit after stop caught: " << e.what() << "\n";
  } catch (...) {
    std::cout << "submit unknow exception.\n";
  }

  return 0;
}

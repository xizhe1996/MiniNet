#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <vector>

#include "mininet/thread_pool_v3.h"

using mininet::ThreadPool;

int add(int a, int b) { return a + b; }

int main() {
  ThreadPool pool(4);

  auto f1 = pool.submit([] { return 7; });
  auto f2 = pool.submit(add, 3, 4);

  std::cout << "f1 valid=" << f1.valid() << "\n";

  std::cout << "f1=" << f1.get() << "\n";
  std::cout << "f2=" << f2.get() << "\n";

  std::vector<std::future<int>> fs;
  for (int i = 0; i < 10; ++i) {
    fs.push_back(pool.submit([i] { return i * i; }));
  }
  long long sum = 0;
  for (auto& f : fs) sum += f.get();
  std::cout << "sum=" << sum << "\n";

  pool.stop();
  return 0;
}

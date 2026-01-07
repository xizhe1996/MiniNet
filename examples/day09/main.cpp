#include <iostream>
#include <vector>

#include "mininet/join_thread.h"
#include "mininet/thread_safe_counter.h"

using mininet::JoinThread;
using mininet::ThreadSafeCounter;

int main() {
  const int kThreads = 6;
  const int kIters = 100000;

  ThreadSafeCounter counter;

  {
    std::vector<JoinThread> workers;
    workers.reserve(kThreads);

    for (int i = 0; i < kThreads; ++i) {
      workers.emplace_back(std::thread([&counter] {
        for (int i = 0; i < kIters; ++i) counter.increment();
      }));
    }
  }

  std::cout << "expected = " << (int64_t)kThreads * kIters << "\n";
  std::cout << "actual   = " << counter.get() << "\n";

  return 0;
}
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "mininet/join_thread.h"

using mininet::JoinThread;

int main() {
  std::vector<JoinThread> workers;
  workers.reserve(3);

  for (int i = 0; i < 3; ++i) {
    workers.emplace_back(std::thread([i] {
      std::cout << "worker " << i << " start\n";
      std::this_thread::sleep_for(std::chrono::milliseconds(50 * (i + 1)));
      std::cout << "worker " << i << " end\n";
    }));
  }

  std::cout << "main done, leaving scope...\n";
  // workers vector 析构 -> JoinThread 析构 -> 自动 join

  JoinThread jt(std::thread([] { std::cout << "move test.\n"; }));
  JoinThread jt2(std::move(jt));

  return 0;
}

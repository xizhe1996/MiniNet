#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "mininet/blocking_queue_v2.h"
#include "mininet/join_thread.h"

using mininet::BlockingQueue;
using mininet::JoinThread;

int main() {
  BlockingQueue q;

  // consumers
  std::vector<JoinThread> consumers;
  consumers.reserve(2);

  for (int i = 0; i < 2; ++i) {
    consumers.emplace_back(std::thread([&q, i] {
      int x;
      while (q.pop(x)) {
        std::cout << "consumer " << i << " got " << x << "\n";
        // simulate work
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
      std::cout << "consumer " << i << " exit\n";
    }));
  }

  // producer
  JoinThread producer(std::thread([&q] {
    for (int i = 1; i <= 20; ++i) {
      q.push(i);
      std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    q.close();
  }));

  std::cout << "main done\n";
  return 0;
}

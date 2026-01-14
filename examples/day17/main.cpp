#include <chrono>
#include <iostream>
#include <thread>

#include "mininet/bounded_blocking_queue.h"
#include "mininet/join_thread.h"

using mininet::BoundedBlockingQueue;
using mininet::JoinThread;

int main() {
  BoundedBlockingQueue<int> que(2);

  JoinThread consumer(std::thread([&] {
    int x;
    while (que.pop(x)) {
      std::cout << "consumer: " << x << "\n";
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
      if (x == 4) {
        que.close();
        std::cout << "que closed\n";
      }
    }
  }));

  JoinThread producer(std::thread([&] {
    for (int i = 0; i < 5; ++i) {
      std::cout << "[producer] pushing " << i << "\n";
      que.push(i);
      std::cout << "[producer] pushed " << i << "\n";
    }
    std::cout << "[producer] done\n";
  }));

  std::cout << "[main] done\n";
  return 0;
}
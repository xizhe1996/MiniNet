#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "mininet/blocking_queue_v3.h"
#include "mininet/join_thread.h"

using mininet::BlockingQueue;
using mininet::JoinThread;

struct Task {
  std::string name;
  std::vector<int> payload;

  Task() = default;
  Task(std::string n, std::vector<int> p)
      : name(std::move(n)), payload(std::move(p)) {}
};

int main() {
  BlockingQueue<Task> q;

  // consumer
  JoinThread consumer(std::thread([&q] {
    Task t;
    while (q.pop(t)) {
      std::cout << "consume: " << t.name << " payload_size=" << t.payload.size()
                << "\n";
    }
    std::cout << "consumer exit\n";
  }));

  // producer
  JoinThread producer(std::thread([&q] {
    for (int i = 0; i < 5; ++i) {
      std::vector<int> payload(1000 * (i + 1), i);
      q.emplace("task" + std::to_string(i), std::move(payload));
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    q.close();
  }));

  std::cout << "main done\n";
  return 0;
}

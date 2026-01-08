#include <iostream>
#include <thread>
#include <vector>

#include "mininet/blocking_queue.h"
#include "mininet/join_thread.h"

using mininet::BlockingQueue;
using mininet::JoinThread;

int main() {
  BlockingQueue bq;

  std::vector<JoinThread> consumers;
  consumers.reserve(2);

  for (int i = 0; i < 2; ++i) {
    consumers.emplace_back(std::thread([&bq, i] {
      while (true) {
        int value = bq.pop();
        if (value != -1) {
          std::cout << "consumers " << i << ": get value: " << value << "\n";
        } else {
          std::cout << "consumers " << i << ": break.\n";
          break;
        }
      }
    }));
  }

  JoinThread producer(std::thread([&bq] {
    for (int i = 0; i < 20; ++i) {
      bq.push(i);
    }
    bq.push(-1);
    bq.push(-1);
  }));

  std::cout << "main done.\n";
  return 0;
}
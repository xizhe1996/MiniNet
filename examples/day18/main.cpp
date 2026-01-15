// examples/day18/main.cpp
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "mininet/thread_pool_v4.h"

using mininet::BoundedBlockingQueue;
using mininet::ThreadPool;

int main() {
  ThreadPool pool(/*nThreads=*/2, /*queueCapacity=*/2);

  // 1) try_submit：打满队列后应返回 false
  {
    std::atomic<int> ran{0};

    auto slowTask = [&] {
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
      ran.fetch_add(1);
    };

    bool ok1 = pool.try_submit(slowTask);
    bool ok2 = pool.try_submit(slowTask);
    bool ok3 = pool.try_submit(slowTask);
    bool ok4 = pool.try_submit(slowTask);

    std::cout << "[try_submit] ok1=" << ok1 << " ok2=" << ok2 << " ok3=" << ok3
              << " ok4=" << ok4 << "\n";

    // TODO: 等待一会，让任务跑完
    std::this_thread::sleep_for(std::chrono::milliseconds(800));
    std::cout << "[try_submit] ran=" << ran.load() << "\n";
  }

  // 2) submit：即使队列小，也能通过阻塞提交最终完成
  {
    std::vector<std::future<int>> futs;
    for (int i = 0; i < 10; ++i) {
      futs.push_back(pool.submit([i] {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        return i * i;
      }));
    }

    long long sum = 0;
    for (auto& f : futs) {
      sum += f.get();
    }
    std::cout << "[submit] sum=" << sum << "\n";
  }

  // 3) task 抛异常：不能让线程池崩溃
  {
    auto f1 = pool.submit([]() -> int { throw std::runtime_error("boom"); });

    try {
      (void)f1.get();
    } catch (const std::exception& e) {
      std::cout << "[exception] caught: " << e.what() << "\n";
    }

    // 继续提交一个任务，验证池子还活着
    auto f2 = pool.submit([] { return 7; });
    std::cout << "[alive] " << f2.get() << "\n";
  }

  // 4) stop 后 submit 抛异常、try_submit 返回 false
  pool.stop();

  bool ok = pool.try_submit([] {});
  std::cout << "[after stop] try_submit=" << ok << "\n";

  try {
    auto f = pool.submit([] { return 1; });
    std::cout << f.get() << "\n";
  } catch (const std::exception& e) {
    std::cout << "[after stop] submit throws: " << e.what() << "\n";
  }

  return 0;
}

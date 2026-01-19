#include <benchmark/benchmark.h>

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <thread>
#include <vector>

#include "mininet/thread_pool_v4.h"

// tiny work：让任务不是“完全空”，避免 benchmark 只测锁/队列
static inline void tiny_work(int iters = 8) {
  volatile int x = 0;
  for (int i = 0; i < iters; ++i) {
    x += i;
  }
  benchmark::DoNotOptimize(x);
}

// 统一从参数读取：range(0)=threads, range(1)=capacity, range(2)=producers
static inline int get_threads(const benchmark::State& state) {
  return (int)state.range(0);
}
static inline int get_capacity(const benchmark::State& state) {
  return (int)state.range(1);
}
static inline int get_producers(const benchmark::State& state) {
  return (int)state.range(2);
}

// A) submit 吞吐（背压阻塞）：生产者线程不断 submit，benchmark 计时期间统计
// completed
static void BM_Submit_Throughput(benchmark::State& state) {
  const int threads = get_threads(state);
  const int cap = get_capacity(state);
  const int producers = get_producers(state);

  mininet::ThreadPool pool((size_t)threads, (size_t)cap);

  std::atomic<int64_t> completed{0};
  std::atomic<bool> stop{false};

  auto task = [&] {
    // 空任务：尽量测线程池调度/队列开销
    completed.fetch_add(1, std::memory_order_relaxed);
  };

  // producer threads
  std::vector<std::thread> prod;
  prod.reserve(producers);

  // 预热：先塞一点任务，避免 cold start 影响
  for (int i = 0; i < 1000; ++i) {
    pool.submit(task);
  }

  for (int p = 0; p < producers; ++p) {
    prod.emplace_back([&] {
      while (!stop.load(std::memory_order_relaxed)) {
        pool.submit(task);  // 队列满则阻塞（背压）
      }
    });
  }

  for (auto _ : state) {
    // benchmark 计时区间：不做额外操作，仅让 producers 推任务
    benchmark::DoNotOptimize(_);
  }

  // 停止 producers
  stop.store(true, std::memory_order_relaxed);
  for (auto& t : prod) t.join();

  // 等待 drain：给 worker 一点时间执行剩余任务
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  pool.stop();
  const int64_t comp = completed.load(std::memory_order_relaxed);
  state.counters["completed"] = (double)comp;
  state.SetItemsProcessed(comp);
}

// B) try_submit 吞吐 + reject rate：队列满会失败
static void BM_TrySubmit_ThroughputReject(benchmark::State& state) {
  const int threads = get_threads(state);
  const int cap = get_capacity(state);
  const int producers = get_producers(state);

  mininet::ThreadPool pool((size_t)threads, (size_t)cap);

  std::atomic<int64_t> submitted{0};
  std::atomic<int64_t> accepted{0};
  std::atomic<int64_t> completed{0};
  std::atomic<bool> stop{false};

  auto task = [&] { completed.fetch_add(1, std::memory_order_relaxed); };

  // 预热
  for (int i = 0; i < 1000; ++i) {
    (void)pool.try_submit(task);
  }

  std::vector<std::thread> prod;
  prod.reserve(producers);

  for (int p = 0; p < producers; ++p) {
    prod.emplace_back([&] {
      while (!stop.load(std::memory_order_relaxed)) {
        submitted.fetch_add(1, std::memory_order_relaxed);
        if (pool.try_submit(task)) {
          accepted.fetch_add(1, std::memory_order_relaxed);
        }
      }
    });
  }

  for (auto _ : state) {
    benchmark::DoNotOptimize(_);
  }

  stop.store(true, std::memory_order_relaxed);
  for (auto& t : prod) t.join();

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  pool.stop();

  const int64_t sub = submitted.load();
  const int64_t acc = accepted.load();
  const int64_t comp = completed.load();
  const double reject_rate =
      (sub == 0) ? 0.0 : (1.0 - (double)acc / (double)sub);

  state.counters["submitted"] =
      benchmark::Counter((double)sub, benchmark::Counter::kIsRate);
  state.counters["accepted"] =
      benchmark::Counter((double)acc, benchmark::Counter::kIsRate);
  state.counters["completed"] =
      benchmark::Counter((double)comp, benchmark::Counter::kIsRate);
  state.counters["reject_rate"] = reject_rate;

  state.SetItemsProcessed(comp);
}

// C) submit + tiny_work：更接近真实业务（避免过于“空任务”）
static void BM_Submit_WithTinyWork(benchmark::State& state) {
  const int threads = get_threads(state);
  const int cap = get_capacity(state);
  const int producers = get_producers(state);

  mininet::ThreadPool pool((size_t)threads, (size_t)cap);

  std::atomic<int64_t> completed{0};
  std::atomic<bool> stop{false};

  auto task = [&] {
    tiny_work(16);
    completed.fetch_add(1, std::memory_order_relaxed);
  };

  // 预热
  for (int i = 0; i < 1000; ++i) {
    pool.submit(task);
  }

  std::vector<std::thread> prod;
  prod.reserve(producers);

  for (int p = 0; p < producers; ++p) {
    prod.emplace_back([&] {
      while (!stop.load(std::memory_order_relaxed)) {
        pool.submit(task);
      }
    });
  }

  for (auto _ : state) {
    benchmark::DoNotOptimize(_);
  }

  stop.store(true, std::memory_order_relaxed);
  for (auto& t : prod) t.join();

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  pool.stop();

  const int64_t comp = completed.load(std::memory_order_relaxed);
  state.counters["completed"] =
      benchmark::Counter((double)comp, benchmark::Counter::kIsRate);
  state.SetItemsProcessed(comp);
}

static void BM_TrySubmit_OneCall(benchmark::State& state) {
  const int workers = (int)state.range(0);
  const int cap = (int)state.range(1);

  // 共享状态（同一个 benchmark case 的多个线程共享）
  static std::unique_ptr<mininet::ThreadPool> pool;
  static std::once_flag init_once;
  static std::atomic<bool> pool_ready{false};
  static std::atomic<bool> stopping{false};

  static std::atomic<int64_t> submitted{0};
  static std::atomic<int64_t> accepted{0};
  static std::atomic<int64_t> completed{0};

  // 每个 benchmark case 开始时：主线程负责初始化一次
  if (state.thread_index() == 0) {
    // 重置统计
    submitted.store(0, std::memory_order_relaxed);
    accepted.store(0, std::memory_order_relaxed);
    completed.store(0, std::memory_order_relaxed);

    // 每次进入函数都会是新的参数组合，保险起见：先 stop 旧 pool
    if (pool) {
      pool->stop();
      pool.reset();
    }

    pool_ready.store(false, std::memory_order_relaxed);
    stopping.store(false, std::memory_order_relaxed);

    pool = std::make_unique<mininet::ThreadPool>((size_t)workers, (size_t)cap);
    pool_ready.store(true, std::memory_order_release);
  } else {
    // 其他线程等待 pool_ready
    while (!pool_ready.load(std::memory_order_acquire)) {
      // 自旋
    }
  }

  auto task = [] { completed.fetch_add(1, std::memory_order_relaxed); };

  for (auto _ : state) {
    benchmark::DoNotOptimize(_);
    submitted.fetch_add(1, std::memory_order_relaxed);
    if (pool->try_submit(task)) {
      accepted.fetch_add(1, std::memory_order_relaxed);
    }
  }

  // 收尾：主线程 stop pool 并输出 counters
  if (state.thread_index() == 0) {
    stopping.store(true, std::memory_order_release);
    // 给 worker 一点时间 drain（否则 completed 可能偏低）
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    pool->stop();
    pool.reset();

    const int64_t sub = submitted.load(std::memory_order_relaxed);
    const int64_t acc = accepted.load(std::memory_order_relaxed);
    const int64_t comp = completed.load(std::memory_order_relaxed);
    const double reject_rate =
        (sub == 0) ? 0.0 : (1.0 - (double)acc / (double)sub);

    state.counters["reject_rate"] = reject_rate;
    state.counters["submitted"] =
        benchmark::Counter((double)sub, benchmark::Counter::kIsRate);
    state.counters["accepted"] =
        benchmark::Counter((double)acc, benchmark::Counter::kIsRate);
    state.counters["completed"] =
        benchmark::Counter((double)comp, benchmark::Counter::kIsRate);

    state.SetItemsProcessed(comp);
  } else {
    // 非主线程：等主线程 stop 完再返回，避免 use-after-free
    while (!stopping.load(std::memory_order_acquire)) {
    }
  }
}

// 参数矩阵：threads, capacity, producers
static void ApplyArgs(benchmark::internal::Benchmark* b) {
  const int thread_cases[] = {1, 2, 4};
  const int cap_cases[] = {2, 64, 1024};
  const int prod_cases[] = {1, 2, 4};

  for (int th : thread_cases) {
    for (int cap : cap_cases) {
      for (int prod : prod_cases) {
        b->Args({th, cap, prod});
      }
    }
  }
}

BENCHMARK(BM_Submit_Throughput)->Apply(ApplyArgs);
BENCHMARK(BM_TrySubmit_ThroughputReject)->Apply(ApplyArgs);
BENCHMARK(BM_Submit_WithTinyWork)->Apply(ApplyArgs);
BENCHMARK(BM_TrySubmit_OneCall)
    ->Args({1, 2})
    ->Args({1, 64})
    ->Args({1, 1024})
    ->Args({2, 2})
    ->Args({2, 64})
    ->Args({2, 1024})
    ->Args({4, 2})
    ->Args({4, 64})
    ->Args({4, 1024})
    ->Threads(1)
    ->Threads(2)
    ->Threads(4)
    ->MinTime(0.5);

BENCHMARK_MAIN();

#include <algorithm>  // minmax_element
#include <chrono>
#include <iostream>
#include <numeric>
#include <vector>

class ScopedTimer {
 public:
  ScopedTimer(const char* name)
      : name_(name), start_(std::chrono::steady_clock::now()) {
    std::cout << "[Start] " << name_ << "\n";
  }

  ~ScopedTimer() noexcept {
    auto end = std::chrono::steady_clock::now();
    auto ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start_)
            .count();
    std::cout << "[End] " << name_ << " cost " << ms << " ms\n";
  }

 private:
  const char* name_;
  std::chrono::steady_clock::time_point start_;
};

// 任务1：把 argv 里的数字解析成 vector<int>
std::vector<int> parse_numbers(int argc, char* argv[]) {
  std::vector<int> nums;
  // skip name
  for (int i = 1; i < argc; ++i) {
    int x = std::stoi(argv[i]);
    nums.push_back(x);
  }
  return nums;
}

void print_stats(const std::vector<int>& nums) {
  if (nums.empty()) {
    std::cout << "nums is empty!" << "\n";
    return;
  }

  std::cout << "count: " << nums.size() << "\n";

  long long sum = std::accumulate(nums.begin(), nums.end(), 0LL);
  std::cout << "sum: " << sum << "\n";

  auto [minIt, maxIt] = std::minmax_element(nums.begin(), nums.end());
  std::cout << "min: " << *minIt << "\n";
  std::cout << "max: " << *maxIt << "\n";

  double avg = sum / static_cast<double>(nums.size());
  std::cout << "avg: " << avg << "\n";
}

int main(int argc, char* argv[]) {
  ScopedTimer timer("day02_stats");

  auto nums = parse_numbers(argc, argv);
  if (nums.empty()) {
    std::cout << "Usage: ./day02 1 2 3 4\n";
    return 0;
  }

  print_stats(nums);
  return 0;
}

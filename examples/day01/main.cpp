#include <chrono>
#include <iostream>
#include <thread>

class ScopedTimer {
 public:
  ScopedTimer(const char* name)
      : name_(name), start_time_(std::chrono::steady_clock::now()) {
    std::cout << "Timer [" << name_ << "] started." << std::endl;
  }

  ~ScopedTimer() noexcept {
    auto end = std::chrono::steady_clock::now();
    auto ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start_time_)
            .count();
    std::cout << "[End] " << name_ << " cost " << ms << " ms\n";
  }

 private:
  const char* name_;
  std::chrono::steady_clock::time_point start_time_;
};

int main(int argc, char* argv[]) {
  ScopedTimer timer("day01_demo");
  std::cout << " Hello MiniNet!" << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(120));
  std::cout << "Done" << std::endl;
  return 0;
}
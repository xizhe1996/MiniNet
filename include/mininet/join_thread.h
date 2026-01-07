#pragma once
#include <thread>

namespace mininet {

class JoinThread {
public:
    // TODO 1: 构造函数，接收 std::thread&&
    explicit JoinThread(std::thread&& t);

    // TODO 2: 析构函数，joinable 时 join
    ~JoinThread() noexcept;

    // TODO 3: 禁止拷贝
    JoinThread(const JoinThread&) = delete;
    JoinThread& operator=(const JoinThread&) = delete;

    // TODO 4: 允许移动（可选，但建议做）
    JoinThread(JoinThread&& other) noexcept;
    JoinThread& operator=(JoinThread&& other) noexcept;

    // TODO 5: 获取内部 thread
    std::thread& get() { return t_; }
    const std::thread& get() const { return t_; }

private:
    std::thread t_;
};

} // namespace mininet

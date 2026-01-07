# Week01 学习总结（Day01 ~ Day07）— MiniNet

> 本周目标：恢复 C++ 动手能力 + 建立工程化习惯 + 实现网络基础组件 Buffer（具备 readv/write 能力）
> 输出形式：可复习的工程周报 + 面试卡片（Flashcards）
> 环境：Linux + g++ + VSCode Remote SSH + C++17

---

## 0. 本周成果一览（面试表达版）

本周我在 Linux 环境下用 C++17 从零搭建并持续演进 MiniNet 项目，完成：

* **RAII 计时器 ScopedTimer**：用于输出代码块耗时，建立性能意识
* **命令行统计工具**：练习 argc/argv、vector、accumulate、minmax_element
* **网络核心组件 Buffer（读写索引模型）**：

  * append / peek / retrieve / retrieve_all_as_string
  * 空洞复用（compact）：减少 resize/realloc
  * 使用 `readv` 从 fd 高性能读取到 Buffer（scatter read）
  * 使用 `write` 将 Buffer 数据写入 fd，并消费已写数据
* **CMake 工程化构建**：构建 mininet 库 + 自动构建所有 dayXX examples
* **build.sh 脚本与 .gitignore**：提升工程体验，保持仓库干净

> 可用于面试项目描述：
> “实现了一个网络 Buffer，采用读写索引设计，支持 compact 复用空间，并实现 readv/write 的 fd IO 接口，为后续 epoll Reactor 与 TCP server 打基础。”

---

## 1. 每日任务回顾（Day-by-Day）

### Day01：ScopedTimer + Linux 编译运行

**目标**：跑通 C++17 编译流程，完成 RAII 小模块
**产出**：

* `ScopedTimer` 构造开始计时，析构输出耗时
* 了解 `std::chrono::steady_clock` 与 `sleep_for`
  **关键点**：
* `steady_clock` 单调递增，适合测耗时
* 析构函数建议 `noexcept`（工程习惯）

---

### Day02：argv 解析 + 统计计算

**目标**：从输入到输出的基本后端编程能力恢复
**产出**：

* `parse_numbers(argc, argv)`：解析数字参数到 vector
* `print_stats(nums)`：输出 count/sum/min/max/avg
  **关键点**：
* `std::stoi` 转换字符串
* `std::accumulate(..., 0LL)` 防止溢出（返回类型由初值决定）
* `std::minmax_element` 一次遍历求 min/max
* 函数内部自保（空输入检查）

---

### Day03：Buffer v1（读写索引模型）

**目标**：实现网络开发中最核心的数据结构之一
**产出**：`Buffer` 类初版

* `append` / `peek` / `readable_bytes`
* `retrieve(n)` / `retrieve_all()` / `retrieve_all_as_string()`
  **关键点**：
* 使用 `vector<char>` + `read_idx_/write_idx_`
* 可读范围：`[read_idx_, write_idx_)`
* `retrieve` 只移动读指针，不 erase（避免 O(n)）

---

### Day04：Buffer 空洞复用（compact / make_space）

**目标**：解决 retrieve 之后前部空洞无法复用导致频繁扩容的问题
**产出**：

* `make_space(len)`：优先 compact，再 resize
* 使用 `memmove` 把 readable 数据挪到头部
  **关键点**：
* `writable_bytes < len` 才需要腾空间
* 如果 `read_idx_ + writable_bytes >= len` → compact
* 否则 resize 到 `write_idx_ + len`

---

### Day05：Buffer read_fd（readv 高性能读取）

**目标**：让 Buffer 具备读取 fd（socket/pipe）的能力
**产出**：

* `read_fd(int fd, int* saved_errno)`
* `begin_write()` / `has_written(len)` 支持直接写入 writable 区
* 用 `pipe()` 编写稳定测试，并模拟粘包（多次 write）
  **关键点**：
* `readv` scatter read：一次 syscall 读到 Buffer + extrabuf
* extrabuf 常用 64KB（减少 syscall、栈上空间可接受）
* 返回 `ssize_t`：可能是 -1（错误）
* 保存 `errno` 到 `saved_errno`，供上层判断（EAGAIN/EINTR 等）

---

### Day06：Buffer write_fd（写回 fd）

**目标**：让 Buffer 支持输出缓冲（output buffer）
**产出**：

* `write_fd(int fd, int* saved_errno)`
* 写入后 `retrieve(n)` 消费已写数据
* pipe test：write_fd → read back 验证一致
  **关键点**：
* `write()` 可能 short write（写不完），这是正常的
* 设计成 “单次尝试写”：

  * 写了多少消费多少
  * 剩余数据留给下次 write_fd
* 后续在非阻塞 socket 上依赖 EPOLLOUT 重试（未来 reactor 会用）

---

### Day07：CMake 工程化构建 + build.sh + .gitignore

**目标**：把项目变成真正可维护工程
**产出**：

* 根目录 `CMakeLists.txt`
* 构建 `mininet` 静态库 + 所有 examples 可执行文件
* `tools/build.sh`：一键 configure + build
* `.gitignore` 忽略 build/ 等编译产物
  **关键点**：
* `cmake -S . -B build` 配置
* `cmake --build build -j` 编译（并行）
* `set -e`：命令失败立即退出（脚本工程习惯）
* build/ 必须 gitignore（避免仓库污染）

---

## 2. 本周核心知识点（语法 + STL + Linux）

### C++ 语法与标准库

* **RAII**：资源生命周期绑定对象生命周期（ScopedTimer / fd wrapper 未来可用）
* `std::chrono`：

  * `steady_clock` vs `system_clock`
  * `duration_cast`
* `std::vector`：

  * `size()` 与 `capacity()` 区别（写 buffer 时必须明确）
* `std::accumulate`：

  * 初始值决定返回类型（0LL 防溢出）
* `std::minmax_element`：

  * 一次遍历求 min/max
* `std::copy` / `std::memmove`：

  * memmove 支持重叠内存移动（compact 使用）

### Linux / 系统调用

* fd 抽象：file / socket / pipe 都是 fd
* `pipe()`：构建可控测试环境
* `readv()`：一次 syscall 读多个 buffer（scatter read）
* `write()`：可能 short write
* `errno`：系统调用出错原因，需要保存供上层处理
* `ssize_t`：读写返回值可为负

### 工程化

* CMake target：

  * `add_library` / `add_executable`
  * `target_include_directories`
  * `target_link_libraries`
* build out-of-source：build/ 与源码分离
* `.gitignore` 管理产物与 IDE 文件
* 小步迭代：先能跑 → 再 correctness → 再优化

---

## 3. 本周工程习惯与踩坑记录（非常重要）

### 3.1 size vs capacity 的坑

* 写 Buffer 时必须基于 `size()` 判断可写空间
* `capacity()` 只是预留，不代表可写范围

### 3.2 Buffer 的接口要“自洽”

* 即使 main 做了空检查，函数内部仍应处理 empty 输入
* make_space 必须包含 `writable >= len` early return

### 3.3 Debug 日志的取舍

* 临时 debug 可加 cout
* 提交前应删除或用宏控制（避免污染库）

### 3.4 工程推进方式

* 看得懂但写不出是正常阶段
* 有效方法：

  * 写之前读前一天代码 5~10 分钟
  * 先写骨架（声明 + 空实现）
  * 再填最小可运行逻辑
  * 最后补边界和优化

---

## 4. 面试复习卡片（Flashcards：Q/A）

### Buffer / 网络方向

**Q1：为什么 Buffer 需要 read_idx/write_idx？**
A：字节流读取是增量的，读写索引可以 O(1) 维护 readable/writable 区间，retrieve 仅移动读指针，避免 O(n) erase。

**Q2：为什么 retrieve 不用 vector::erase？**
A：erase 会搬移元素，复杂度 O(n)，高频调用会导致性能差；retrieve 只移动 read_idx，O(1)。

**Q3：什么是 compact？什么时候触发？**
A：retrieve 后前部产生空洞。append 时若 writable 不够，但前部空洞 + writable 足够，则把 readable 数据 memmove 到前面复用空间，避免 resize。

**Q4：为什么 read_fd 使用 readv？**
A：readv 能一次 syscall 把数据读到两块 buffer：Buffer writable + extrabuf，减少 syscall 和频繁 resize，提高吞吐。

**Q5：为什么 extrabuf 常用 64KB？**
A：64KB 是常见 IO 缓冲大小，能减少系统调用次数，栈上分配开销可接受；网络库常用这个级别。

**Q6：write 为什么可能 short write？怎么处理？**
A：非阻塞 socket、pipe buffer 满等情况下 write 可能只写一部分。处理方式：写多少 retrieve 多少，剩余数据保留在 output buffer，等待下一次可写事件继续写。

**Q7：errno 有什么用？为什么要保存？**
A：系统调用失败时设置 errno（如 EAGAIN/EINTR）。保存到 saved_errno 交给上层决定重试/等待/报错。

---

### C++ / STL 基础

**Q8：steady_clock 和 system_clock 区别？**
A：steady_clock 单调递增不受系统时间调整影响，适合计时；system_clock 表示墙上时间可能被 NTP 调整。

**Q9：accumulate 的返回类型由什么决定？**
A：由初始值类型决定。用 0LL 可让结果为 long long，防止 int 溢出。

**Q10：vector 的 size 和 capacity 区别？**
A：size 是当前可用元素数（合法访问范围），capacity 是已分配容量（不代表已初始化可访问），写 buffer 时必须以 size 为准。

---

### 工程化

**Q11：为什么 build/ 应该加入 .gitignore？**
A：build 目录包含生成文件和中间产物，不同机器不同配置会不同，提交会污染仓库并造成冲突。

**Q12：set -e 的作用？**
A：脚本任何命令失败立即退出，避免后续命令在错误状态下继续执行导致难以定位问题。

---

## 5. 下周计划（Week02：Day08~Day14，预览）

下周将从“网络 Buffer 基础”转向“并发与线程池基础”（面试高频）：

* Day08：`JoinThread`（thread RAII）
* Day09：mutex + lock_guard + 线程安全计数
* Day10：condition_variable + producer-consumer
* Day11：多消费者 + graceful shutdown（close/stop）
* Day12：ThreadSafeQueue（支持 close）
* Day13：ThreadPool v1（void task）
* Day14：ThreadPool v2（future + packaged_task）

> 同时：Buffer 会继续补齐更工程的接口（如 prepend / find CRLF 等），为后续 TCP/HTTP 做准备（可选）。

---

## 6. 本周文件清单（便于复盘）

* `include/mininet/buffer.h`
* `src/buffer.cpp`
* `examples/day01~day06/main.cpp`
* `CMakeLists.txt`
* `tools/build.sh`
* `.gitignore`
* `docs/week01.md`（本文件）

---

> ✅ 本周复盘总结：
> 已完成从“能编译运行”到“能实现网络核心 Buffer 并具备 readv/write 接口”的跃迁，工程化构建已搭好，为后续并发与网络服务器开发打下基础。

---


好的。下面是一份**“一口气可直接复制提交”的完整 Markdown 文档**，**已经把所有需要的内容都合并进一个 md 文件中**：

* Day8–Day19 全部内容
* 线程池最终设计与代码语义
* 语法卡（含示例）
* 设计取舍与面试高频
* 测试结果
* 对应 git message

你**直接整体复制 → 新建 md 文件 → git add / commit 即可**。

---

````md
# Week2：并发与线程池（Day8 – Day19）

> 项目背景：  
> 本项目是一个使用 C++ 构建的高性能网络服务框架练习项目，目标面向 **机器人 / 芯片 / AI / Linux / 网络 / 系统方向岗位**。  
>  
> 本阶段（Day8–Day19）专注于 **并发基础与线程池的工程化实现**，为后续 Reactor、epoll、多线程网络与性能分析打下基础。

---

## 一、阶段目标（Day8 – Day19）

本阶段的核心目标不是“写一个能跑的线程池”，而是：

- 理解 **并发组件的职责边界**
- 明确 **线程池的语义设计**
- 掌握 **C++ 并发与异步结果传递**
- 能够在面试中解释：
  - 为什么这样设计
  - 不这样设计会有什么问题
  - 如何验证与调优

线程池在机器人控制、AI 推理后端、芯片仿真、系统服务中都非常常见，因此这是一个**基础但高频的系统级模块**。

---

## 二、组件演进路线（Day8 → Day19）

### 1. JoinThread（Day8–Day9）

**作用**
- 对 `std::thread` 的 RAII 封装
- 析构时自动 join，避免忘记 join 导致 `std::terminate`

**关键点**
- 线程生命周期必须与对象生命周期绑定
- join 可能阻塞，线程函数必须设计为可退出

**工程意义**
- 防止资源泄漏
- 防止程序异常终止

---

### 2. BlockingQueue（Day10–Day11）

**作用**
- 基于 `mutex + condition_variable` 的阻塞队列
- 解决生产者 / 消费者同步问题

**核心机制**
- 队列空时，消费者阻塞
- 队列非空时，唤醒消费者

**关键实现点**
```cpp
cv.wait(lock, [&]{ return !queue.empty(); });
````

**注意事项**

* 使用谓词版本 `wait` 防止虚假唤醒
* 临界区内只做必要操作

---

### 3. BoundedBlockingQueue（Day12–Day14）

**新增能力**

* 队列容量上限（Bounded）
* `not_full` / `not_empty` 两个条件变量
* `close()` 语义

**核心接口**

* `push`：队列满则阻塞（背压）
* `try_push`：不阻塞，满则失败
* `pop`：队列空则阻塞，close 且空时返回 false

**设计意义**

* 防止任务无限堆积
* 支持限流、快速失败
* 在机器人 / 芯片 / 边缘系统中尤为重要

---

### 4. ThreadPool（Day15–Day19）

#### 4.1 线程模型

* 固定数量 worker 线程
* 任务队列：`BoundedBlockingQueue<std::function<void()>>`
* worker loop：

  * pop 任务
  * 执行任务
  * 捕获异常，防止线程崩溃

---

#### 4.2 提交接口设计

```cpp
bool try_submit(std::function<void()> task);

template<class F, class... Args>
std::future<R> submit(F&& f, Args&&... args);
```

**接口语义**

* `try_submit`

  * 不阻塞
  * 队列满 / stop / close → 返回 false
  * 适合限流、降级路径

* `submit`

  * 阻塞（背压）
  * 返回 `future`
  * stop 后抛异常

---

#### 4.3 任务封装（核心技术点）

**目标**

* 支持任意可调用对象
* 支持返回值 / void
* 支持 move-only 参数
* 支持异常传播

**实现要点**

* `std::packaged_task` + `std::future`
* `shared_ptr` 延长任务生命周期
* `tuple + std::apply` 替代 `std::bind`
* `if constexpr` 处理 `void` 返回类型

```cpp
auto taskPtr = std::make_shared<std::packaged_task<R()>>(
  [func = std::forward<F>(f),
   tup  = std::make_tuple(std::forward<Args>(args)...)]() mutable -> R {
    if constexpr (std::is_void_v<R>) {
      std::apply(std::move(func), std::move(tup));
      return;
    } else {
      return std::apply(std::move(func), std::move(tup));
    }
  }
);
```

---

## 三、关键语义与边界（面试重点）

### 1. close vs stop

* **close（队列层面）**

  * 不再接收新任务
  * 唤醒等待线程
  * 允许已入队任务 drain

* **stop（线程池层面）**

  * 幂等（CAS）
  * 调用 `tasks_.close()`
  * join 所有 worker

---

### 2. 为什么需要有界队列？

* 无界队列在高负载下会：

  * 内存无限增长
  * 延迟不可控
* 有界队列提供 backpressure
* 是系统稳定性的基础

---

### 3. 为什么 worker 必须捕获异常？

* 未捕获异常会导致线程直接 `std::terminate`
* 线程池可能“悄悄瘫痪”
* 正确做法：

  * worker 捕获异常
  * 通过 `future.get()` 传播给提交方

---

## 四、语法卡（含示例）

### 1. condition_variable 谓词等待

```cpp
cv.wait(lock, [&]{ return closed || !queue.empty(); });
```

---

### 2. atomic + CAS（幂等 stop）

```cpp
bool expected = false;
if (!stopped.compare_exchange_strong(expected, true)) return;
```

---

### 3. packaged_task + future

```cpp
std::packaged_task<int()> pt([]{ return 42; });
auto fut = pt.get_future();
pt();
int x = fut.get(); // 42
```

---

### 4. tuple + apply

```cpp
auto tup = std::make_tuple(1, 2);
int r = std::apply([](int a, int b){ return a + b; }, tup);
```

---

### 5. if constexpr（处理 void）

```cpp
if constexpr (std::is_void_v<R>) {
  func();
  return;
}
```

---

### 6. shared_ptr 的作用（线程池场景）

* 延长任务生命周期（跨 submit 返回）
* 保证 lambda 可拷贝，兼容 `std::function`
* 避免悬空引用与未定义行为

---

## 五、测试与验证（Day19）

### 覆盖场景

* `future<void>` 正确返回
* move-only 参数（`std::unique_ptr`）支持
* 队列满时 `try_submit` 失败
* stop 后：

  * `try_submit` 返回 false
  * `submit` 抛异常
* task 抛异常但线程池继续工作

### 示例输出

```
[void] ok
[move-only] 42
[after stop] try_submit=0
[after stop] submit throws: threadpool stopped.
```

---

## 六、常见面试问题与回答要点

**Q：为什么不用无界队列？**
A：无界队列会导致内存膨胀和延迟失控，有界队列提供 backpressure，是系统稳定性的基础。

**Q：为什么用 shared_ptr？**
A：任务在 submit 返回后仍需执行，shared_ptr 延长生命周期并保证任务可拷贝以兼容 `std::function`。

**Q：stop 和 close 的区别？**
A：close 是队列语义，stop 是线程池语义；stop 会触发 close 并等待 worker 退出。

**Q：异常如何处理？**
A：worker 捕获异常，异常通过 future 传播给调用方。

---

## 七、本阶段产出与 Git 提交

### 本阶段完成内容

* JoinThread
* BlockingQueue
* BoundedBlockingQueue（含 try_push / close）
* ThreadPool（有界队列 + future + void + move-only）
* 完整测试用例

### 对应 Git Message

```
docs: add week2 concurrency and thread pool summary (day8-day19)
```

---

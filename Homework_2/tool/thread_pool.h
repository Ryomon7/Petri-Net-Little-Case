//
// Created by LvMeng on 2022/11/5 19:51.
// Note: This file is used to……
// Copyright (C) 2022 * Ltd. All rights reserved.
//

#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_
#include <atomic>// 原子操作库
#include <chrono>// 时间库
#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

using std::cout;
using std::endl;

namespace tool {
class ThreadPool {
public:
  using PoolSeconds = std::chrono::seconds;

  // 线程池结构体
  // core_threads: 核心线程个数，线程池中最少拥有的线程个数，初始化就会创建好的线程，常驻于线程池
  // max_threads: >=core_threads，当任务的个数太多线程池执行不过来时，
  //              内部就会创建更多的线程用于执行更多的任务，内部线程数不会超过max_threads
  // max_task_size: 内部允许存储的最大任务个数，未使用
  // time_out: Cache线程的超时时间，Cache线程指的是max_threads-core_threads的线程,
  //          当time_out时间内没有执行任务，此线程就会被自动回收
  struct ThreadPoolConfig {
    int core_threads;
    int max_threads;
    int max_task_size;
    PoolSeconds time_out;
  };
  // 线程的状态枚举：包括等待、运行、停止
  enum class ThreadState {
    kInit = 0,
    kWaiting,
    kRunning,
    kStop
  };
  // 线程的种类标识：标志该线程是核心线程还是Cache线程
  // Cache线程是内部为执行更多任务临时创建出来的
  enum class ThreadFlag {
    kInit = 0,
    kCore,
    kCache
  };
  using ThreadPtr = std::shared_ptr<std::thread>;
  using ThreadId = std::atomic<int>;
  using ThreadStateAtomic = std::atomic<ThreadState>;
  using ThreadFlagAtomic = std::atomic<ThreadFlag>;
  // 线程池中线程存在的基本单位，每个线程都有个自定义的ID，有线程种类标识和状态
  struct ThreadWrapper {
    ThreadPtr ptr;// 线程的指针
    ThreadId id{}; // 线程的id
    ThreadStateAtomic state{};// 线程的状态
    ThreadFlagAtomic flag{};// 线程的种类
    ThreadWrapper() {
        ptr = nullptr;
        id = 0;
        state.store(ThreadState::kInit);
    }
  };
  using ThreadWrapperPtr = std::shared_ptr<ThreadWrapper>;
  using ThreadPoolLock = std::unique_lock<std::mutex>;
  // 线程池构造函数
  explicit ThreadPool(ThreadPoolConfig config) : config_(config) {
      this->total_function_num_.store(0);
      this->waiting_thread_num_.store(0);
      this->thread_id_.store(0);
      this->is_shutdown_.store(false);
      this->is_shutdown_now_.store(false);
      if (IsValidConfig(config_)) {
          is_available_.store(true);
      } else {
          is_available_.store(false);
      }
  }

  ~ThreadPool() { ShutDown(); }

  // 重新设置线程池（主要目的是重新定义核心线程数量）
  bool Reset(ThreadPoolConfig config) {
      if (!IsValidConfig(config)) {
          return false;
      }
      if (config_.core_threads != config.core_threads) {
          return false;
      }
      config_ = config;
      return true;
  }

  // 开启线程池功能
  bool Start() {
      if (!IsAvailable()) {
          return false;
      }
      // 按照配置启动线程池
      int core_thread_num = config_.core_threads;
      //cout << "Init thread num " << core_thread_num << endl;
      while (core_thread_num-- > 0) {// 按序初始化
          AddThread(GetNextThreadId());
      }
      //cout << "Init thread end" << endl;
      return true;
  }

  // 获取正在处于等待状态的线程的个数
  int GetWaitingThreadSize() {
      return this->waiting_thread_num_.load();
  }

  // 获取线程池中当前线程的总个数
  unsigned int GetTotalThreadSize() {
      return this->worker_threads_.size();
  }

  // 放在线程池中执行函数（线程对外接口）
  template<typename F, typename... Args>
  auto Run(F &&f, Args &&... args) -> std::shared_ptr<std::future<std::result_of_t<F(Args...)>>> {
      if (this->is_shutdown_.load() || this->is_shutdown_now_.load() || !IsAvailable()) {
          return nullptr;
      }
      if (GetWaitingThreadSize() == 0 && GetTotalThreadSize() < config_.max_threads) {
          AddThread(GetNextThreadId(), ThreadFlag::kCache);
      }

      using return_type = std::result_of_t<F(Args...)>;
      auto task = std::make_shared<std::packaged_task<return_type()>>(
          std::bind(std::forward<F>(f), std::forward<Args>(args)...));
      total_function_num_++;

      std::future<return_type> res = task->get_future();
      {
          ThreadPoolLock lock(this->task_mutex_);
          this->tasks_.emplace([task]() { (*task)(); });
      }
      this->task_cv_.notify_one();
      return std::make_shared<std::future<std::result_of_t<F(Args...)>>>(std::move(res));
  }

  // 获取当前线程池已经执行过的函数个数
  int GetExecutedFuncNum() { return total_function_num_.load(); }

  // 关掉线程池，内部未执行的任务会继续执行
  void ShutDown() {
      ShutDown(false);
      cout << "shutdown" << endl;
  }

  // 立即关闭线程池，内部未执行的任务直接取消，不会再执行
  void ShutDownNow() {
      ShutDown(true);
      cout << "shutdown now" << endl;
  }

  // 当前线程池是否可用
  bool IsAvailable() {
      return is_available_.load();
  }

private:
  void ShutDown(bool is_now) {
      if (is_available_.load()) {
          if (is_now) {
              this->is_shutdown_now_.store(true);
          } else {
              this->is_shutdown_.store(true);
          }
          this->task_cv_.notify_all();
          is_available_.store(false);
      }
  }

  // 仅使用id，添加core线程
  void AddThread(int id) {
      AddThread(id, ThreadFlag::kCore);
  }

  // 添加线程
  // 线程内部会有一个死循环，不停的等待任务，有任务到来时就会执行，同时内部会判断是否是Cache线程，如果是Cache线程，time_out时间内没有任务执行就会自动退出循环，线程结束。
  //这里还会检查is_shutdown_和is_shutdown_now_标志，根据两个标志位是否为true来判断是否结束线程。
  void AddThread(int id, ThreadFlag thread_flag) {
      // cout << "AddThread: " << id << " flag: " << static_cast<int>(thread_flag) << endl;
      ThreadWrapperPtr thread_ptr = std::make_shared<ThreadWrapper>();
      thread_ptr->id.store(id);
      thread_ptr->flag.store(thread_flag);
      auto func = [this, thread_ptr]() {
        for (;;) {
            std::function<void()> task;
            {
                ThreadPoolLock lock(this->task_mutex_);
                if (thread_ptr->state.load() == ThreadState::kStop) {
                    break;
                }
                // cout << "thread id: " << thread_ptr->id.load() << " running start" << endl;
                thread_ptr->state.store(ThreadState::kWaiting);
                ++this->waiting_thread_num_;
                bool is_timeout = false;
                if (thread_ptr->flag.load() == ThreadFlag::kCore) {
                    this->task_cv_.wait(lock, [this, thread_ptr] {
                      return (this->is_shutdown_ || this->is_shutdown_now_ || !this->tasks_.empty() ||
                          thread_ptr->state.load() == ThreadState::kStop);
                    });
                } else {
                    this->task_cv_.wait_for(lock, this->config_.time_out, [this, thread_ptr] {
                      return (this->is_shutdown_ || this->is_shutdown_now_ || !this->tasks_.empty() ||
                          thread_ptr->state.load() == ThreadState::kStop);
                    });
                    is_timeout = !(this->is_shutdown_ || this->is_shutdown_now_ || !this->tasks_.empty() ||
                        thread_ptr->state.load() == ThreadState::kStop);
                }
                --this->waiting_thread_num_;
                //cout << "thread id: " << thread_ptr->id.load() << " running wait end" << endl;

                if (is_timeout) {
                    thread_ptr->state.store(ThreadState::kStop);
                }

                if (thread_ptr->state.load() == ThreadState::kStop) {
                    //cout << "thread id: " << thread_ptr->id.load() << " state stop" << endl;
                    break;
                }
                if (this->is_shutdown_ && this->tasks_.empty()) {
                    //cout << "thread id: " << thread_ptr->id.load() << " shutdown" << endl;
                    break;
                }
                if (this->is_shutdown_now_) {
                    //cout << "thread id: " << thread_ptr->id.load() << " shutdown now" << endl;
                    break;
                }
                thread_ptr->state.store(ThreadState::kRunning);
                task = std::move(this->tasks_.front());
                this->tasks_.pop();
            }
            task();
        }
        //cout << "thread id: " << thread_ptr->id.load() << " running end" << endl;
      };
      thread_ptr->ptr = std::make_shared<std::thread>(std::move(func));
      if (thread_ptr->ptr->joinable()) {
          thread_ptr->ptr->detach();
      }
      this->worker_threads_.emplace_back(std::move(thread_ptr));
  }

  void Resize(int thread_num) {
      if (thread_num < config_.core_threads) return;
      unsigned int old_thread_num = worker_threads_.size();
      //cout << "old num " << old_thread_num << " resize " << thread_num << endl;
      if (thread_num > old_thread_num) {
          while (thread_num-- > old_thread_num) {
              AddThread(GetNextThreadId());
          }
      } else {
          unsigned int diff = old_thread_num - thread_num;
          auto iter = worker_threads_.begin();
          while (iter != worker_threads_.end()) {
              if (diff == 0) {
                  break;
              }
              auto thread_ptr = *iter;
              if (thread_ptr->flag.load() == ThreadFlag::kCache &&
                  thread_ptr->state.load() == ThreadState::kWaiting) {  // wait
                  thread_ptr->state.store(ThreadState::kStop);          // stop;
                  --diff;
                  iter = worker_threads_.erase(iter);
              } else {
                  ++iter;
              }
          }
          this->task_cv_.notify_all();
      }
  }

  int GetNextThreadId() { return this->thread_id_++; }

  // 判断是配置是否有效
  static bool IsValidConfig(ThreadPoolConfig config) {
      if (config.core_threads < 1 || config.max_threads < config.core_threads || config.time_out.count() < 1) {
          return false;
      }
      return true;
  }

private:
  ThreadPoolConfig config_;// 线程池配置

  std::list<ThreadWrapperPtr> worker_threads_;

  std::queue<std::function<void()>> tasks_;
  std::mutex task_mutex_;
  std::condition_variable task_cv_;

  std::atomic<int> total_function_num_{};
  std::atomic<int> waiting_thread_num_{};
  std::atomic<int> thread_id_{};// 线程池中的线程数量

  std::atomic<bool> is_shutdown_{};// 关闭线程池
  std::atomic<bool> is_shutdown_now_{};// 立刻关闭线程池
  std::atomic<bool> is_available_{};// 线程池是否可用
};
}
#endif //THREAD_POOL_H_

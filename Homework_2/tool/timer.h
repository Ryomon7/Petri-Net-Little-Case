//
// Created by LvMeng on 2022/11/5 19:51.
// Note: This file is used to……
// Copyright (C) 2022 * Ltd. All rights reserved.
//

#ifndef TIMER_H_
#define TIMER_H_
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <queue>
#include "./map_tool.h"
#include "./thread_pool.h"

namespace tool {
class TimerQueue {
public:
  struct InternalS {
    std::chrono::time_point<std::chrono::high_resolution_clock> time_point_;
    std::function<void()> func_;
    int repeated_id;
    bool operator<(const InternalS &b) const { return time_point_ > b.time_point_; }
  };

  //在构造函数中初始化，主要是配置好内部的线程池，线程池中常驻的线程数目前设为4，理论上开多了反而会降低效率。
  TimerQueue() : thread_pool_(tool::ThreadPool::ThreadPoolConfig{4, 4, 40, std::chrono::seconds(4)}) {
      repeated_func_id_.store(0);
      is_running_.store(true);
  }
  ~TimerQueue() { Stop(); }
  // 打开内部的线程池功能，用于执行放入定时器中的任务，同时新开一个线程，循环等待任务到来后送入线程池中执行。
  bool Run() {
      bool ret = thread_pool_.Start();
      if (!ret) {
          return false;
      }
      std::thread([this]() { RunLocal(); }).detach();
      return true;
  }

  // 线程池是否可用
  bool IsAvailable() {
      return thread_pool_.IsAvailable();
  }

  // 返回任务队列的长度
  unsigned int Size() {
      return queue_.size();
  }

  void Stop() {
      is_running_.store(false);
      cond_.notify_all();
      thread_pool_.ShutDown();
  }

  template<typename R, typename P, typename F, typename... Args>
  // 在某段时间后执行任务
  // 根据当前时间加上时间段构造出时间戳从而构造InternalS，放入队列中
  void AddFuncAfterDuration(const std::chrono::duration<R, P> &time, F &&f, Args &&... args) {
      InternalS s;
      s.time_point_ = std::chrono::high_resolution_clock::now() + time;
      s.func_ = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
      std::unique_lock<std::mutex> lock(mutex_);
      queue_.push(s);
      cond_.notify_all();
  }

  template<typename F, typename... Args>
  // 在某一时间点执行任务
  // 根据时间戳构造InternalS，放入队列中
  void AddFuncAtTimePoint(const std::chrono::time_point<std::chrono::high_resolution_clock> &time_point, F &&f,
                          Args &&... args) {
      InternalS s;
      s.time_point_ = time_point;
      s.func_ = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
      std::unique_lock<std::mutex> lock(mutex_);
      queue_.push(s);
      cond_.notify_all();
  }

  template<typename R, typename P, typename F, typename... Args>
  // 重复执行某一任务N次，任务间隔时间T
  // 根据时间戳构造InternalS，放入队列中
  int AddRepeatedFunc(int repeat_num, const std::chrono::duration<R, P> &time, F &&f, Args &&... args) {
      int id = GetNextRepeatedFuncId();
      repeated_id_state_map_.Emplace(id, RepeatedIdState::kRunning);
      auto tem_func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
      AddRepeatedFuncLocal(repeat_num - 1, time, id, std::move(tem_func));
      return id;
  }

  // 内部有repeated_id_state_map 数据结构，用于存储循环任务的ID，
  // 当取消任务执行时，将此ID从repeatedid_state_map中移除，循环任务就会自动取消。
  void CancelRepeatedFuncId(int func_id) {
      repeated_id_state_map_.EraseKey(func_id);
  }

  int GetNextRepeatedFuncId() {
      return repeated_func_id_++;
  }
  enum class RepeatedIdState { // 计时器的状态枚举：包括运行、停止
    kInit = 0,
    kRunning,
    kStop
  };

private:
  void RunLocal() {
      while (is_running_.load()) {
          std::unique_lock<std::mutex> lock(mutex_);
          if (queue_.empty()) {
              cond_.wait(lock);
              continue;
          }
          auto s = queue_.top();
          auto diff = s.time_point_ - std::chrono::high_resolution_clock::now();
          if (std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() > 0) {
              cond_.wait_for(lock, diff);
              continue;
          } else {
              queue_.pop();
              lock.unlock();
              thread_pool_.Run(std::move(s.func_));
          }
      }
  }

  template<typename R, typename P, typename F>
  void AddRepeatedFuncLocal(int repeat_num, const std::chrono::duration<R, P> &time, int id, F &&f) {
      if (!this->repeated_id_state_map_.IsKeyExist(id)) {
          return;
      }
      InternalS s;
      s.time_point_ = std::chrono::high_resolution_clock::now() + time;
      auto tem_func = std::forward<F>(f);
      s.repeated_id = id;
      s.func_ = [this, &tem_func, repeat_num, time, id]() {
        tem_func();
        if (!this->repeated_id_state_map_.IsKeyExist(id) || repeat_num == 0) {
            return;
        }
        AddRepeatedFuncLocal(repeat_num - 1, time, id, std::move(tem_func));
      };
      std::unique_lock<std::mutex> lock(mutex_);
      queue_.push(s);
      lock.unlock();
      cond_.notify_all();
  }

private:
  std::priority_queue<InternalS> queue_; // 定时器任务队列
  std::atomic<bool> is_running_{}; // 定时器是否正在工作
  std::mutex mutex_;
  std::condition_variable cond_;

  tool::ThreadPool thread_pool_;// 定时器线程池

  std::atomic<int> repeated_func_id_{};
  tool::ThreadSafeMap<int, RepeatedIdState> repeated_id_state_map_{};
};
}
#endif //TIMER_H_
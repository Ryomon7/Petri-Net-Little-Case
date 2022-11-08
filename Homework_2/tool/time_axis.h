//
// Created by LvMeng on 2022/11/5 19:51.
// Note: This file is used to……
// Copyright (C) 2022 * Ltd. All rights reserved.
//
#ifndef TIME_AXIS_H_
#define TIME_AXIS_H_
#include <chrono>
#include <iostream>
using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;
class TimeAxis {
  // 时间轴类
private:
  // 时间轴起点
  TimePoint time_origin_;
public:
  // 构造函数，设置时间轴起点
  TimeAxis() : time_origin_(std::chrono::high_resolution_clock::now()) {}
  // 重置时间轴起点
  void Reset() {
      time_origin_ = std::chrono::high_resolution_clock::now();
  }
  // 重设时间轴起点
  void SetTimeOrigin(const TimePoint &time_origin) {
      time_origin_ = time_origin;
  }
  // 返回已经历时间（起点到当前）
  //默认输出毫秒
  long long Elapsed() const {
      return std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::high_resolution_clock::now() - time_origin_).count();
  }
  //微秒
  long long ElapsedMicro() const {
      return std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::high_resolution_clock::now() - time_origin_).count();
  }
  //纳秒
  long long ElapsedNano() const {
      return std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::high_resolution_clock::now() - time_origin_).count();
  }
  //秒
  long long ElapsedSeconds() const {
      return std::chrono::duration_cast<std::chrono::seconds>(
          std::chrono::high_resolution_clock::now() - time_origin_).count();
  }
  //分
  long long ElapsedMinutes() const {
      return std::chrono::duration_cast<std::chrono::minutes>(
          std::chrono::high_resolution_clock::now() - time_origin_).count();
  }
  //时
  long long ElapsedHours() const {
      return std::chrono::duration_cast<std::chrono::hours>(
          std::chrono::high_resolution_clock::now() - time_origin_).count();
  }

  // 重载：返回已经历的时间（双参数，给定起点到当前）
  //默认输出毫秒
  long long Elapsed(TimePoint start_time) const {
      return std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::high_resolution_clock::now() - start_time).count();
  }
  //微秒
  long long ElapsedMicro(TimePoint start_time) const {
      return std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::high_resolution_clock::now() - start_time).count();
  }
  //纳秒
  long long ElapsedNano(TimePoint start_time) const {
      return std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::high_resolution_clock::now() - start_time).count();
  }
  //秒
  long long ElapsedSeconds(TimePoint start_time) const {
      return std::chrono::duration_cast<std::chrono::seconds>(
          std::chrono::high_resolution_clock::now() - start_time).count();
  }
  //分
  long long ElapsedMinutes(TimePoint start_time) const {
      return std::chrono::duration_cast<std::chrono::minutes>(
          std::chrono::high_resolution_clock::now() - start_time).count();
  }
  //时
  long long ElapsedHours(TimePoint start_time) const {
      return std::chrono::duration_cast<std::chrono::hours>(
          std::chrono::high_resolution_clock::now() - start_time).count();
  }
};

#endif //TIME_AXIS_H_

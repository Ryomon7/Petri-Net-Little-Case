//
// Created by LvMeng on 2022/11/6 13:42.
// Note: This file is used to……
// Copyright (C) 2022 * Ltd. All rights reserved.
//

#ifndef TEST_TIMED_PETRI_H_
#define TEST_TIMED_PETRI_H_
#include <iostream>
#include "petri_net.h"
#include "reachability_graph.h"
#include "./tool/timer.h"
#include "tool/time_axis.h"
namespace test_timed_petri {
void test1() {
    // 测试一：定时器tool::timer功能
    tool::TimerQueue q;// 新建一个计时器队列对象
    q.Run();
    for (int i = 20; i < 25; ++i) {
        q.AddFuncAfterDuration(std::chrono::seconds(i + 1),
                               [i]() { std::cout << "this is " << i << " DurationTask" << std::endl; });
        q.AddFuncAtTimePoint(std::chrono::high_resolution_clock::now() + std::chrono::seconds(1),
                             [i]() { std::cout << "this is " << i << " atTask " << std::endl; });
    }
    std::cout << "addTaskFinish" << std::endl;
    int id = q.AddRepeatedFunc(10, std::chrono::seconds(1), []() { std::cout << "RepeatFunc " << std::endl; });
    std::this_thread::sleep_for(std::chrono::seconds(4));
    q.CancelRepeatedFuncId(id);

    std::this_thread::sleep_for(std::chrono::seconds(30));// 手工等待线程执行完毕
    q.Stop();
}
void test2() {
    // 测试二：测试时间轴TimeAxis功能
    TimeAxis t;
    std::string temp;
    std::cout << "please input sth.: " << std::endl;
    std::cin >> temp;
    long long s = t.ElapsedSeconds();
    int n = static_cast<int>(s);
    std::cout << "time Elapsed:" << n << " s" << std::endl;
    long long ms = t.Elapsed();
    std::cout << "time Elapsed:" << ms << " ms" << std::endl;

    // 相当于转化成了时间戳
    std::cout << "high_resolution_clock: "
              << std::chrono::high_resolution_clock::to_time_t(std::chrono::high_resolution_clock::now()) << endl;
}
void test3() {
    // 测试三：测试“机械臂”附时petri网构造函数
    Eigen::VectorXi p(7);
    Eigen::VectorXi t(3);
    Eigen::MatrixXi i(7, 3);
    Eigen::MatrixXi o(7, 3);
    Eigen::VectorXi m_0(7);
    Eigen::VectorXi place_delay(7);

    for (int num_of_place = 0; num_of_place < 7; num_of_place++) {
        p[num_of_place] = num_of_place;
    }
    for (int num_of_trans = 0; num_of_trans < 3; num_of_trans++) {
        t[num_of_trans] = num_of_trans;
    }
    i << 1, 0, 0,
        1, 0, 0,
        1, 0, 0,
        0, 0, 1,
        0, 1, 0,
        0, 1, 0,
        0, 0, 0;
    o << 0, 1, 0,
        0, 1, 0,
        0, 0, 1,
        0, 0, 1,
        1, 0, 0,
        1, 0, 0,
        0, 1, 0;
    m_0 << 0, 0, 0, 1, 1, 1, 0;
//    m_0 << 1, 1, 1, 1, 0, 0, 0;
    place_delay << 10, 8, 0, 12, 10, 8, 0;
    auto *timed_net = new PlaceTimedPetriNet(p, t, i, o, m_0, place_delay);
    std::cout << "m_0:\n" << timed_net->GetMarking() << std::endl;
    std::vector<int> t_to_fire = timed_net->GetFirableTransition(); // 验证GetFirableTransition
    if (t_to_fire.empty()) {
        std::cout << "sorry Ryomon, there is no trans can be fire" << std::endl;
    } else {
        for (auto k : t_to_fire) {
            std::cout << "\ntrans_can_fire: " << k << std::endl;
        }
    }
}
void test_try_fail() {
    // 这里是被我否决掉的timed peri net方案的timedPetri的类和实现
    /****头文件-类******/
    class PlaceTimedPetriNet : public PetriNet {
    private:
      TimeAxis time_axis_;// 时间轴
      std::chrono::time_point<std::chrono::high_resolution_clock> time_stamp_;// 时间戳
      Eigen::VectorXi place_delay_;// 库所时延（客观配置）
      Eigen::VectorXi token_wait_time_;

      bool FreshFirableTransition();// （遮盖）刷新可用激发--m
      bool FreshTokenWaitTime();// 刷新Token等待时间--v
      bool FreshTimeStamp();// 刷新时间戳--g
    public:
      const Eigen::VectorXi &GetTokenWaitTime() const;
      PlaceTimedPetriNet(Eigen::VectorXi &p,
                         Eigen::VectorXi &t,
                         Eigen::MatrixXi &i,
                         Eigen::MatrixXi &o,
                         Eigen::VectorXi &m_0,
                         Eigen::VectorXi &place_delay);
      // 设置时间轴起点，时间校准
      bool SetTimeAxisOrigin(const std::chrono::time_point<std::chrono::high_resolution_clock> &time_origin) {
          time_axis_.SetTimeOrigin(time_origin);
          return PETRI_NET_SUCCESS;
      }
      // （遮盖）激发一个transition，并改变petri net的状态
    };

    /******源文件**实现********/
    /*const Eigen::VectorXi &PlaceTimedPetriNet::GetTokenWaitTime() const {
        return token_wait_time_;
    }
    PlaceTimedPetriNet::PlaceTimedPetriNet(Eigen::VectorXi &p,
                                           Eigen::VectorXi &t,
                                           Eigen::MatrixXi &i,
                                           Eigen::MatrixXi &o,
                                           Eigen::VectorXi &m_0,
                                           Eigen::VectorXi &place_delay) : PetriNet(p, t, i, o, m_0),
        place_delay_(place_delay) {
        time_axis_.Reset();// 启动时间轴
        token_wait_time_ = Eigen::VectorXi::Zero(num_of_place_);// token滞留时间
    }
    bool PlaceTimedPetriNet::FreshTokenWaitTime() {
        for (int i = 0; i < num_of_place_; i++) {

        }
        return PETRI_NET_SUCCESS;
    }*/
}
void test4() {
    // 测试给eigen::vectorXi排序
    Eigen::VectorXi vec(4);
    vec << 3, 1, 2, 4;
    std::cout << "vec before: \n" << vec << std::endl;
    std::sort(vec.data(), vec.data() + vec.size());
    std::cout << "sorted vec: \n" << vec << std::endl;

    Eigen::VectorXi Rm = Eigen::VectorXi::Constant(3,7);
    std::cout << "build a vector use a constant: \n" << Rm << std::endl;
}
void test5() {
    // 测试calc下一个激发的时间
    // std::cout << "int max: " << MAX_INT << std::endl;
    Eigen::VectorXi p(7);
    Eigen::VectorXi t(3);
    Eigen::MatrixXi i(7, 3);
    Eigen::MatrixXi o(7, 3);
    Eigen::VectorXi m_0(7);
    Eigen::VectorXi place_delay(7);

    for (int num_of_place = 0; num_of_place < 7; num_of_place++) {
        p[num_of_place] = num_of_place;
    }
    for (int num_of_trans = 0; num_of_trans < 3; num_of_trans++) {
        t[num_of_trans] = num_of_trans;
    }
    i << 1, 0, 0,
        1, 0, 0,
        1, 0, 0,
        0, 0, 1,
        0, 1, 0,
        0, 1, 0,
        0, 0, 0;
    o << 0, 1, 0,
        0, 1, 0,
        0, 0, 1,
        0, 0, 1,
        1, 0, 0,
        1, 0, 0,
        0, 1, 0;
//    m_0 << 0, 0, 0, 1, 1, 1, 0;
    m_0 << 1, 1, 1, 1, 0, 0, 0;
    place_delay << 10, 8, 0, 12, 10, 8, 0;
    auto *timed_net = new PlaceTimedPetriNet(p, t, i, o, m_0, place_delay);
    std::cout << "m_0: " << timed_net->GetMarking().transpose() << std::endl;
    std::cout << "Next fire need time: " << timed_net->GetNextFirableInformation().first << std::endl;
    std::cout << "The future transition will be: " << timed_net->GetNextFirableInformation().second << std::endl;
}
void test6(){
    // 测试设置stamp（跳转到最近的激发时间）以后，某些参数刷新是否正常
    Eigen::VectorXi p(7);
    Eigen::VectorXi t(3);
    Eigen::MatrixXi i(7, 3);
    Eigen::MatrixXi o(7, 3);
    Eigen::VectorXi m_0(7);
    Eigen::VectorXi place_delay(7);

    for (int num_of_place = 0; num_of_place < 7; num_of_place++) {
        p[num_of_place] = num_of_place;
    }
    for (int num_of_trans = 0; num_of_trans < 3; num_of_trans++) {
        t[num_of_trans] = num_of_trans;
    }
    i << 1, 0, 0,
        1, 0, 0,
        1, 0, 0,
        0, 0, 1,
        0, 1, 0,
        0, 1, 0,
        0, 0, 0;
    o << 0, 1, 0,
        0, 1, 0,
        0, 0, 1,
        0, 0, 1,
        1, 0, 0,
        1, 0, 0,
        0, 1, 0;
    m_0 << 0, 0, 0, 1, 1, 1, 0;
//    m_0 << 1, 1, 1, 1, 0, 0, 0;
    place_delay << 10, 8, 0, 12, 10, 8, 0;
    auto *timed_net = new PlaceTimedPetriNet(p, t, i, o, m_0, place_delay);
    std::cout << "m_0: " << timed_net->GetMarking().transpose() <<"\n"<< std::endl;

    /*当前：
     * 1. 当前是否有可激发的变迁，他们分别是哪个变迁？
     * 2. 多久以后可以有下一个可激发的变迁？他是谁？
     */
    // 1
    std::vector<int> t_to_fire = timed_net->GetFirableTransition();
    if (t_to_fire.empty()) {
        std::cout << "sorry Ryomon, there is no trans can be fire" << std::endl;
    } else {
        for (auto k : t_to_fire) {
            std::cout << "trans_can_fire_now: " << k << std::endl;
        }
    }
    // 2
    std::cout << "Next fire need time: " << timed_net->GetNextFirableInformation().first << std::endl;
    std::cout << "The future transition will be: " << timed_net->GetNextFirableInformation().second << std::endl;

    // 设置pn的时间戳
    std::cout << std::endl;
    std::pair<int,int> next_time_pair = timed_net->GetNextFirableInformation();
    timed_net->SetTimeStamp(next_time_pair.first);

    /* 跳转后
     * 1. 跳转以后是否有可激发的变迁，他们分别是哪个变迁？
     * 2. 多久以后可以有下一个可激发的变迁？他是谁？
     */
    // 1

    Eigen::VectorXi  making_after = timed_net -> GetMarking();
    std::cout << "making_after: " << making_after.transpose() << std::endl;
    Eigen::VectorXi wait_time_after = timed_net->GetTokenWaitTime();
    std::cout << "wait_time_after: " << wait_time_after.transpose() << std::endl;
    std::vector<int> t_to_fire_after = timed_net->GetFirableTransition();
    if (t_to_fire_after.empty()) {
        std::cout << "sorry Ryomon, there is no trans can be fire" << std::endl;
    } else {
        for (auto k : t_to_fire_after) {
            std::cout << "trans_can_fire_now: " << k << std::endl;
        }
    }
    // 2
    std::cout << "\nNext fire need time: " << timed_net->GetNextFirableInformation().first << std::endl;
    std::cout << "The future transition will be: " << timed_net->GetNextFirableInformation().second << std::endl;
}
void test7(){
    // 激活一个变迁，各项指标是否正常
    // std::cout << "int max: " << MAX_INT << std::endl;
    Eigen::VectorXi p(7);
    Eigen::VectorXi t(3);
    Eigen::MatrixXi i(7, 3);
    Eigen::MatrixXi o(7, 3);
    Eigen::VectorXi m_0(7);
    Eigen::VectorXi place_delay(7);

    for (int num_of_place = 0; num_of_place < 7; num_of_place++) {
        p[num_of_place] = num_of_place;
    }
    for (int num_of_trans = 0; num_of_trans < 3; num_of_trans++) {
        t[num_of_trans] = num_of_trans;
    }
    i << 1, 0, 0,
        1, 0, 0,
        1, 0, 0,
        0, 0, 1,
        0, 1, 0,
        0, 1, 0,
        0, 0, 0;
    o << 0, 1, 0,
        0, 1, 0,
        0, 0, 1,
        0, 0, 1,
        1, 0, 0,
        1, 0, 0,
        0, 1, 0;
    m_0 << 0, 0, 0, 1, 1, 1, 0;
//    m_0 << 1, 1, 1, 1, 0, 0, 0;
    place_delay << 10, 8, 0, 12, 10, 8, 0;
    auto *timed_net = new PlaceTimedPetriNet(p, t, i, o, m_0, place_delay);

    for(int time = 0; time < 7; time++){
        std::cout << "m_"<<time << ": "<< timed_net->GetMarking().transpose() << std::endl;
        std::cout << "v_"<<time << ": "<< timed_net->GetTokenWaitTime().transpose() << std::endl;
        std::cout << "g_"<<time << ": "<< timed_net->GetTimeStamp()<< std::endl;
        std::cout << "Next fire need time: " << timed_net->GetNextFirableInformation().first << std::endl;
        std::cout << "The future transition will be: " << timed_net->GetNextFirableInformation().second << std::endl;
        timed_net->FiringATransition(timed_net->GetNextFirableInformation().second, timed_net->GetNextFirableInformation().first);
        std::cout << "Firing end!\n" << std::endl;
    }
}
}
#endif //TEST_TIMED_PETRI_H_

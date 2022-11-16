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
#include "timed_reachability_graph.h"
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

    Eigen::VectorXi Rm = Eigen::VectorXi::Constant(3, 7);
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
void test6() {
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
    std::cout << "m_0: " << timed_net->GetMarking().transpose() << "\n" << std::endl;

    /* ****************************
     * 当前：
     * 1. 当前是否有可激发的变迁，他们分别是哪个变迁？
     * 2. 多久以后可以有下一个可激发的变迁？他是谁？
     * ****************************/
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
    std::pair<int, int> next_time_pair = timed_net->GetNextFirableInformation();
    timed_net->FastForward(next_time_pair.first);

    /* 跳转后
     * 1. 跳转以后是否有可激发的变迁，他们分别是哪个变迁？
     * 2. 多久以后可以有下一个可激发的变迁？他是谁？
     */
    // 1

    Eigen::VectorXi making_after = timed_net->GetMarking();
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
void test7() {
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

    for (int time = 0; time < 40; time++) {
        std::cout << "g_" << time << ": " << timed_net->GetTimeStamp() << std::endl;
        std::cout << "m_" << time << ": " << timed_net->GetMarking().transpose() << std::endl;
        std::cout << "v_" << time << ": " << timed_net->GetTokenWaitTime().transpose() << std::endl;
        std::vector<int> t_to_fire = timed_net->GetFirableTransition();
        if (t_to_fire.empty()) {
            std::cout << "sorry Ryomon, there is no trans can be fire" << std::endl;
        } else {
            for (auto k : t_to_fire) {
                std::cout << "************** trans_can_fire_now **************: " << k << " ";
            }
            std::cout << std::endl;
        }
        std::cout << "Next fire need time: " << timed_net->GetNextFirableInformation().first << std::endl;
        std::cout << "The future transition will be: " << timed_net->GetNextFirableInformation().second << std::endl;
        timed_net->FiringATransition(timed_net->GetNextFirableInformation().second,
                                     timed_net->GetNextFirableInformation().first);
        std::cout << "Firing end!\n" << std::endl;
    }
}
void test8() {
    // 建立可达图----这是一个失败的可达图（因为开发到这一步还无法回退状态）
    // std::cout << "int max: " << MAX_INT << std::endl;
    Eigen::VectorXi p(7);
    Eigen::VectorXi t(3);
    Eigen::MatrixXi i(7, 3);
    Eigen::MatrixXi o(7, 3);
    Eigen::VectorXi m_0(7);
    Eigen::VectorXi place_delay(7);

    Eigen::VectorXi goal_making(7);

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
    goal_making << 0, 0, 0, 0, 0, 0, 10;
    auto *timed_net = new PlaceTimedPetriNet(p, t, i, o, m_0, place_delay);

    // 生成可达图并展示
    timed::ReachabilityGraph r_graph;
    bool res = r_graph.BuildReachabilityGraph(timed_net, goal_making);
    if (!res) {
        std::cout << "build graph fail.." << std::endl;
    } else {
        // 展示可达图 //TODO: 是不是可以写到可达图类中
        std::vector<timed::MarkingNode> nodes = r_graph.GetNodes();
        unsigned int len = nodes.size();
        for (int kk = 0; kk < len; kk++) {
            std::cout << "node(" << nodes[kk].node_name_ << ")\n";
            std::cout << "marking_" << kk << ": " << nodes[kk].timed_petri_net_info_.marking_info.transpose()
                      << std::endl;
            for (auto nn : nodes[kk].transition_to_son_) {
                std::cout << "  --[T_" << nn.first << "]--> node(" << nn.second << ")\n";
            }
            std::cout << std::endl;
        }
    }
}
void test9() {
    // 测试 timed petri net 的 reset 功能是否正常
    // 测试TimedPN：构造 -> 显示 -> 激发 -> 显示 -> 回退 -> 显示

    // 1. 构造PN
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
    //m_0 << 1, 1, 1, 1, 0, 0, 0;
    place_delay << 10, 8, 0, 12, 10, 8, 0;
    auto *timed_net = new PlaceTimedPetriNet(p, t, i, o, m_0, place_delay);
    // 2. 显示
    //显示什么？stamp, m, wait_time, entry_time, now_firable_trans, stamp
    std::cout << "initial status" << std::endl;
    int g_time_0 = timed_net->GetTimeStamp();
    std::cout << "time tamp g_0: " << g_time_0 << std::endl;
    Eigen::VectorXi making_0 = timed_net->GetMarking();
    std::cout << "making: " << making_0.transpose() << std::endl;
    Eigen::VectorXi wait_time_0 = timed_net->GetTokenWaitTime();
    std::cout << "token wait time : " << wait_time_0.transpose() << std::endl;
    Eigen::VectorXi entry_time_0 = timed_net->GetTokenEntryTime();
    std::cout << "token entry time: " << entry_time_0.transpose() << std::endl;
    std::vector<int> t_to_fire_0 = timed_net->GetFirableTransition();
    if (t_to_fire_0.empty()) {
        std::cout << "sorry Ryomon, there is no trans can be fire now" << std::endl;
    } else {
        for (auto k : t_to_fire_0) {
            std::cout << "trans_can_fire_now: " << k << std::endl;
        }
    }
    // 3. 保存状态和激发
    // 3.1 保存状态
    auto *tp = new TimedPetriNetInfo{timed_net->GetTimeStamp(),
                                     timed_net->GetMarking(),
                                     timed_net->GetTokenWaitTime(),
                                     timed_net->GetTokenEntryTime()};
    std::cout << "new TimedPetriNetInfo finish" << std::endl;
    // 3.2 激发
    timed_net->FiringATransition(timed_net->GetNextFirableInformation().second,
                                 timed_net->GetNextFirableInformation().first);
    std::cout << "\nFiring end!\n" << std::endl;
    // 4. 显示
    std::cout << "After Fire status" << std::endl;
    int g_time_1 = timed_net->GetTimeStamp();
    std::cout << "time tamp g_1: " << g_time_1 << std::endl;
    Eigen::VectorXi making_1 = timed_net->GetMarking();
    std::cout << "making: " << making_1.transpose() << std::endl;
    Eigen::VectorXi wait_time_1 = timed_net->GetTokenWaitTime();
    std::cout << "token wait time : " << wait_time_1.transpose() << std::endl;
    Eigen::VectorXi entry_time_1 = timed_net->GetTokenEntryTime();
    std::cout << "token entry time: " << entry_time_1.transpose() << std::endl;
    std::vector<int> t_to_fire_1 = timed_net->GetFirableTransition();
    if (t_to_fire_1.empty()) {
        std::cout << "sorry Ryomon, there is no trans can be fire now" << std::endl;
    } else {
        for (auto k : t_to_fire_1) {
            std::cout << "trans_can_fire_now: " << k << std::endl;
        }
    }
    // 5. 回退
    timed_net->ResetTimedPetriNet(*tp);
    std::cout << "\nReset end!\n" << std::endl;
    // 6. 显示
    std::cout << "After Reset status" << std::endl;
    int g_time_2 = timed_net->GetTimeStamp();
    std::cout << "time tamp g_2: " << g_time_2 << std::endl;
    Eigen::VectorXi making_2 = timed_net->GetMarking();
    std::cout << "making: " << making_2.transpose() << std::endl;
    Eigen::VectorXi wait_time_2 = timed_net->GetTokenWaitTime();
    std::cout << "token wait time : " << wait_time_2.transpose() << std::endl;
    Eigen::VectorXi entry_time_2 = timed_net->GetTokenEntryTime();
    std::cout << "token entry time: " << entry_time_2.transpose() << std::endl;
    std::vector<int> t_to_fire_2 = timed_net->GetFirableTransition();
    if (t_to_fire_2.empty()) {
        std::cout << "sorry Ryomon, there is no trans can be fire now" << std::endl;
    } else {
        for (auto k : t_to_fire_2) {
            std::cout << "trans_can_fire_now: " << k << std::endl;
        }
    }

    // 再激发，再显示
    timed_net->FiringATransition(timed_net->GetNextFirableInformation().second,
                                 timed_net->GetNextFirableInformation().first);
    std::cout << "\nFiring twice end!\n" << std::endl;
    // 4. 显示
    std::cout << "After Fire status" << std::endl;
    int g_time_3 = timed_net->GetTimeStamp();
    std::cout << "time tamp g_3: " << g_time_3 << std::endl;
    Eigen::VectorXi making_3 = timed_net->GetMarking();
    std::cout << "making: " << making_3.transpose() << std::endl;
    Eigen::VectorXi wait_time_3 = timed_net->GetTokenWaitTime();
    std::cout << "token wait time : " << wait_time_3.transpose() << std::endl;
    Eigen::VectorXi entry_time_3 = timed_net->GetTokenEntryTime();
    std::cout << "token entry time: " << entry_time_3.transpose() << std::endl;
    std::vector<int> t_to_fire_3 = timed_net->GetFirableTransition();
    if (t_to_fire_3.empty()) {
        std::cout << "sorry Ryomon, there is no trans can be fire now" << std::endl;
    } else {
        for (auto k : t_to_fire_3) {
            std::cout << "trans_can_fire_now: " << k << std::endl;
        }
    }
}
// 生成可达图，会显示节点细节
void test10() {
    Eigen::VectorXi p(7);
    Eigen::VectorXi t(3);
    Eigen::MatrixXi i(7, 3);
    Eigen::MatrixXi o(7, 3);
    Eigen::VectorXi m_0(7);
    Eigen::VectorXi place_delay(7);

    Eigen::VectorXi goal_making(7);

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
    goal_making << 0, 0, 0, 0, 0, 0, 10;
    auto *timed_net = new PlaceTimedPetriNet(p, t, i, o, m_0, place_delay);

    // 生成可达图并展示
    timed::ReachabilityGraph r_graph;
    bool res = r_graph.BuildReachabilityGraph(timed_net, goal_making);
    if (!res) {
        std::cout << "build graph fail.." << std::endl;
    } else {
        // 展示可达图 //TODO: 是不是可以写到可达图类中
        std::vector<timed::MarkingNode> nodes = r_graph.GetNodes();
        unsigned int len = nodes.size();
        for (int kk = 0; kk < len; kk++) {
            std::cout << "node(" << nodes[kk].node_name_ << ")\n";
            std::cout << "time stamp: " << nodes[kk].timed_petri_net_info_.time_stamp_info << std::endl;
            std::cout << "marking_" << kk << ": " << nodes[kk].timed_petri_net_info_.marking_info.transpose()
                      << std::endl;
            std::cout << "token wait time : " << nodes[kk].timed_petri_net_info_.token_wait_time_info.transpose()
                      << std::endl;
            std::cout << "token entry time: " << nodes[kk].timed_petri_net_info_.token_entry_time_info.transpose()
                      << std::endl;
            std::vector<int> t_to_fire_3 = timed_net->GetFirableTransition();

            for (auto nn : nodes[kk].transition_to_son_) {
                std::cout << "  ---[T_" << nn.first << "]---> node(" << nn.second << ")\n";
            }
            std::cout << "\n" << std::endl;
        }
    }
}
// 生成可达图, 主要展示节点关系
void test11() {

    Eigen::VectorXi p(7);
    Eigen::VectorXi t(3);
    Eigen::MatrixXi i(7, 3);
    Eigen::MatrixXi o(7, 3);
    Eigen::VectorXi m_0(7);
    Eigen::VectorXi place_delay(7);

    Eigen::VectorXi goal_making(7);

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
    goal_making << 0, 0, 0, 0, 0, 0, 10;
    auto *timed_net = new PlaceTimedPetriNet(p, t, i, o, m_0, place_delay);

    // 生成可达图并展示
    timed::ReachabilityGraph r_graph;
    bool res = r_graph.BuildReachabilityGraph(timed_net, goal_making);
    if (!res) {
        std::cout << "build graph fail." << std::endl;
    } else {
        // 展示可达图 //TODO: 是不是可以写到可达图类中
        std::vector<timed::MarkingNode> nodes = r_graph.GetNodes();
        unsigned int len = nodes.size();
        for (int kk = 0; kk < len; kk++) {
            std::cout << "node(" << nodes[kk].node_name_ << ")\n";
            std::cout << "marking_" << kk << ": " << nodes[kk].timed_petri_net_info_.marking_info.transpose()
                      << std::endl;
            std::vector<int> t_to_fire_3 = timed_net->GetFirableTransition();
            for (auto nn : nodes[kk].transition_to_son_) {
                std::cout << "  ---[T_" << nn.first << "]---> node(" << nn.second << ")\n";
            }
            std::cout << std::endl;
        }
    }
}
}
#endif //TEST_TIMED_PETRI_H_

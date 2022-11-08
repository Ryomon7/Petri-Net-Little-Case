//
// Created by LvMeng on 2022/11/6 13:38.
// Note: This file is used to……
// Copyright (C) 2022 * Ltd. All rights reserved.
//

#ifndef TOOL_TEST_PETRI_H_
#define TOOL_TEST_PETRI_H_
#include <iostream>
#include "petri_net.h"
#include "reachability_graph.h"
namespace test_petri {
void test1() {
    // 测试一：构造一个普通矩阵，并用它来构造petri网
    Eigen::VectorXi p(3);
    Eigen::VectorXi t(2);
    Eigen::MatrixXi i(3, 2);
    Eigen::MatrixXi o(3, 2);
    Eigen::VectorXi m_0(3);

    for (int num_of_place = 0; num_of_place < 3; num_of_place++) {
        p[num_of_place] = num_of_place + 1;
    }
    m_0 << 2, 1, 0;
    for (int num_of_trans = 0; num_of_trans < 2; num_of_trans++) {
        t[num_of_trans] = num_of_trans + 1;
    }
    i << 2, 0, 1, 0, 0, 2;
    o << 0, 2, 0, 1, 2, 0;
    PetriNet *net = new PetriNet(p, t, i, o, m_0);
    std::cout << "m_0:\n" << net->GetMarking() << std::endl;
    std::cout << "m_0.size()\n" << m_0.size() << std::endl;
    std::cout << "m_0_transpose\n" << m_0.transpose() << std::endl;// 转置
}
void test2() {
    // 测试二：显示matrix的行向量列向量
    Eigen::MatrixXi i(3, 4);
    i << 2, 0, 1, 0, 0, 1, 0, 3, 0, 1, 0, 2;
    std::cout << "matrix:\n" << i << std::endl;
    std::cout << "second row:\n" << i.row(1) << std::endl;
    std::cout << "second column:\n" << i.col(1) << std::endl;

    Eigen::SparseMatrix<int> t = i.sparseView();
    std::cout << "sparse matrix:\n" << t << std::endl;
    Eigen::MatrixXi t_(3, 4);
    t_ << 3, 0, 1, 0, 0, 1, 0, 3, 0, 1, 0, 2;
    std::cout << "matrix minus:\n" << t - t_ << std::endl;
    Eigen::VectorXi second_colum = t.col(1);
    std::cout << "second column:\n" << second_colum << std::endl;
    Eigen::Vector3i zero = Eigen::Vector3i::Zero();
    std::cout << "zero:\n" << zero << std::endl;
}
void test3() {
    // 测试如何用sparse matrix
    Eigen::SparseMatrix<double> A(5, 5);
    std::vector<Eigen::Triplet<double> > triplets;
    // 初始化非零元位置,r表示行，c表示列，默认列优先。
    int r[4] = {2, 0, 1, 3};
    int c[4] = {0, 1, 3, 4};
    double value[] = {1, 1, 2, 3}; //给非零元赋值
    for (int i = 0; i < 4; i++)
        triplets.emplace_back(r[i], c[i], value[i]);
    //初始化矩阵
    A.setFromTriplets(triplets.begin(), triplets.end());

    std::cout << "A=" << std::endl << A << "end..." << std::endl;
    std::cout << "the second row: " << std::endl << A.row(1) << std::endl;
}
void test4() {
    // 测试petri网的GetFirableTransition、FiringATransition、SetMarking方法
    Eigen::VectorXi p(3);
    Eigen::VectorXi t(2);
    Eigen::MatrixXi i(3, 2);
    Eigen::MatrixXi o(3, 2);
    Eigen::VectorXi m_0(3);

    for (int num_of_place = 0; num_of_place < 3; num_of_place++) {
        p[num_of_place] = num_of_place + 1;
    }
    m_0 << 2, 1, 0;
    for (int num_of_trans = 0; num_of_trans < 2; num_of_trans++) {
        t[num_of_trans] = num_of_trans + 1;
    }
    i << 2, 0, 1, 0, 0, 2;
    o << 0, 2, 0, 1, 2, 0;
    PetriNet *net = new PetriNet(p, t, i, o, m_0);
    std::cout << "\nm_0:\n" << net->GetMarking() << std::endl;
    std::vector<int> t_to_fire = net->GetFirableTransition(); // 验证GetFirableTransition
    for (auto k : t_to_fire) {
        std::cout << "\nk: " << k << std::endl;
    }
    net->FiringATransition(t_to_fire[0]); // 验证FiringATransition
    std::cout << "\nm_1:\n" << net->GetMarking() << std::endl;
    std::vector<int> t_to_fire_1 = net->GetFirableTransition();
    for (auto k : t_to_fire_1) {
        std::cout << "\nk: " << k << std::endl;
    }
    net->SetMarking(m_0);// 验证SetMarking
    std::vector<int> t_to_fire_2 = net->GetFirableTransition();
    std::cout << "\nafter set making:" << std::endl;
    for (auto k : t_to_fire_2) {
        std::cout << "k: " << k << std::endl;
    }
}
void test5() {
    // 测试五：构造一个稍微大点的petri网，看看效果
    Eigen::VectorXi p(5);
    Eigen::VectorXi t(4);
    Eigen::MatrixXi i(5, 4);
    Eigen::MatrixXi o(5, 4);
    Eigen::VectorXi m_0(5);

    for (int num_of_place = 0; num_of_place < 5; num_of_place++) {
        p[num_of_place] = num_of_place;
    }
    m_0 << 0, 0, 1, 0, 0;
    for (int num_of_trans = 0; num_of_trans < 4; num_of_trans++) {
        t[num_of_trans] = num_of_trans;
    }
    i << 1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
        0, 0, 0, 1;
    o << 0, 0, 0, 1,
        1, 0, 0, 0,
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0;
    PetriNet *net = new PetriNet(p, t, i, o, m_0);
    std::cout << "m_0:\n" << net->GetMarking() << std::endl;
    std::vector<int> t_to_fire = net->GetFirableTransition();
    for (auto k : t_to_fire) {
        std::cout << "k: " << k << std::endl;
    }
    net->FiringATransition(t_to_fire[0]);
    std::cout << "\nm_1:\n" << net->GetMarking() << std::endl;
    std::vector<int> t_to_fire_1 = net->GetFirableTransition();
    for (auto k : t_to_fire_1) {
        std::cout << "k: " << k << std::endl;
    }
    Eigen::VectorXi marking = net->GetMarking();
    std::vector<int> arg(&marking[0], marking.data() + marking.cols() * marking.rows());
    std::cout << "change to vector: vector.size()=" << arg.size() << std::endl;
    for (int z : arg) {
        std::cout << z << ", ";
    }
    std::string strData;
    for (auto data : arg) {
        strData += std::to_string(data) + ",";
    }
    strData = strData.substr(0, strData.size() - 1);
    std::cout << "\nhello this is string_out: " << strData << std::endl;
}
// 测试六：生成”碗“可达图
void test6() {
    Eigen::VectorXi p(5);
    Eigen::VectorXi t(4);
    Eigen::MatrixXi i(5, 4);
    Eigen::MatrixXi o(5, 4);
    Eigen::VectorXi m_0(5);

    for (int num_of_place = 0; num_of_place < 5; num_of_place++) {
        p[num_of_place] = num_of_place;
    }
    m_0 << 1, 0, 0, 0, 0;
    for (int num_of_trans = 0; num_of_trans < 4; num_of_trans++) {
        t[num_of_trans] = num_of_trans;
    }
    i << 1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
        0, 0, 0, 1;
    o << 0, 0, 0, 1,
        1, 0, 0, 0,
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0;
    PetriNet *net = new PetriNet(p, t, i, o, m_0);
    // 生成可达图并展示
    ReachabilityGraph r_graph;
    bool res = r_graph.BuildReachabilityGraph(net);
    if (!res) {
        std::cout << "build graph fail.." << std::endl;
    } else {
        // 展示可达图 //TODO: 是不是可以写到可达图类中
        std::vector<MarkingNode> nodes = r_graph.GetNodes();
        unsigned int len = nodes.size();
        for (int kk = 0; kk < len; kk++) {
            std::cout << "node(" << nodes[kk].node_name_ << ")\n";
            std::cout << "marking_" << kk << ": " << nodes[kk].marking_.transpose() << std::endl;
            for (auto nn : nodes[kk].transition_to_son_) {
                std::cout << "  --[t_" << nn.first << "]--> node(" << nn.second << ")\n";
            }
            std::cout << std::endl;
        }
    }
}
void test7() {
    // 测试怎么使用动态矩阵, 怎么变形
    Eigen::MatrixXi m(4, 4);
    Eigen::MatrixXi n(3, 1);
    m << 1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
        13, 14, 15, 16;
    n << 6, 6, 6;
    std::cout << "m(4*4):\n" << m << std::endl;
    std::cout << "m(4*4) size:" << m.size() << std::endl;
    m = n;
    std::cout << "m(3*1):\n" << m << std::endl;
    std::cout << "m(3*1) size:" << m.size() << std::endl;
    std::vector<std::pair<int, int>> transition_to_son_;
    std::cout << "initial std::vector size:" << transition_to_son_.size() << std::endl;
    Eigen::VectorXi marking_;
    std::cout << "initial Eigen::VectorXi size:" << marking_.size() << std::endl;

}
void test8() {
    // 测试怎么修改hush_map
    std::unordered_map<std::string, std::string> u_map{
        {"Python", "www.python.com"},
        {"Java", "www.java.com"},
        {"Linux", "www.linux.com"}};
    //查找成功
    auto iter = u_map.find("Python");
    std::cout << iter->first << ":  " << iter->second << std::endl;
    iter->second = "hello python";
    std::cout << "\nchange result: " << std::endl;
    for (auto &vote : u_map) {
        std::cout << vote.first << ": " << vote.second << std::endl;
    }
    std::cout << std::endl;
}
// 测试九：生成”H2O可达图“
void test9() {
    Eigen::VectorXi p(3);
    Eigen::VectorXi t(2);
    Eigen::MatrixXi i(3, 2);
    Eigen::MatrixXi o(3, 2);
    Eigen::VectorXi m_0(3);
    for (int num_of_place = 0; num_of_place < 3; num_of_place++) {
        p[num_of_place] = num_of_place + 1;
    }
    m_0 << 2, 1, 2;
    for (int num_of_trans = 0; num_of_trans < 2; num_of_trans++) {
        t[num_of_trans] = num_of_trans + 1;
    }
    i << 2, 0, 1, 0, 0, 2;
    o << 0, 2, 0, 1, 2, 0;
    PetriNet *net = new PetriNet(p, t, i, o, m_0);
    // 生成可达图并展示
    ReachabilityGraph r_graph;
    bool res = r_graph.BuildReachabilityGraph(net);
    if (!res) {
        std::cout << "build graph fail.." << std::endl;
    } else {
        // 展示可达图 //TODO: 是不是可以写到可达图类中
        std::vector<MarkingNode> nodes = r_graph.GetNodes();
        unsigned int len = nodes.size();
        for (int kk = 0; kk < len; kk++) {
            std::cout << "node(" << nodes[kk].node_name_ << ")\n";
            std::cout << "marking_" << kk << ": " << nodes[kk].marking_.transpose() << std::endl;
            for (auto nn : nodes[kk].transition_to_son_) {
                std::cout << "  --[T_" << nn.first << "]--> node(" << nn.second << ")\n";
            }
            std::cout << std::endl;
        }
    }
}
}
#endif //TOOL_TEST_PETRI_H_

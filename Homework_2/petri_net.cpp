//
// Created by LvMeng on 2022/9/29 20:53.
// Note: This file is used to write implementations
//
#include <vector>
#include "petri_net.h"
// 含参构造函数
PetriNet::PetriNet(Eigen::VectorXi &p, Eigen::VectorXi &t,
                   Eigen::MatrixXi &i, Eigen::MatrixXi &o,
                   Eigen::VectorXi &m_0) {
    place_ = p;
    transition_ = t;
    num_of_place_ = p.size();
    num_of_trans_ = t.size();
    // 采用稀疏矩阵来表示input、output、trans，优点：节省存储空间
    input_matrix_ = i.sparseView();
    output_matrix_ = o.sparseView();
    trans_matrix_ = (o - i).sparseView();
    // 初始状态就是m0
    marking_ = m_0;
    FreshFirableTransition();
}
// 获取当前的marking
const Eigen::VectorXi &PetriNet::GetMarking() const {
    return marking_;
}
// 设置marking
void PetriNet::SetMarking(const Eigen::VectorXi &marking) {
    marking_ = marking;
    FreshFirableTransition();
}
// 获取可用激发
const std::vector<int> &PetriNet::GetFirableTransition() const {
    return firable_transition_;
}
// 更新当前可用激发：marking改变后皆需调用此函数
bool PetriNet::FreshFirableTransition() {
    Eigen::VectorXi vector_temp(num_of_place_);
    std::vector<int> firable_trans;
    for (int i = 0; i < num_of_trans_; i++) {
        vector_temp = marking_ - input_matrix_.col(i);
        bool flag = true;
        //TODO:此处可优化，改用稀疏矩阵判断是否可激发
        for (auto &j : vector_temp) {
            flag = (j >= 0) & flag;
        }
        if (flag) firable_trans.emplace_back(i);
    }
    firable_transition_ = firable_trans;
    return PETRI_NET_SUCCESS;
}
// 激发一个transition，并改变petri net的marking
bool PetriNet::FiringATransition(const int t) {
    //保护机制，防止随便传数字，需要check一下是不是可继发
    bool is_in_firable_vec = false;
    for (auto i : firable_transition_) {
        if (t == i) is_in_firable_vec = true;
    }
    if (is_in_firable_vec) {
        // 如果为可激发的transition才激发petri网 -> 改变marking
        marking_ = marking_ + trans_matrix_.col(t);
        FreshFirableTransition();
        return PETRI_NET_SUCCESS;
    }
    return PETRI_NET_FAIL;
}


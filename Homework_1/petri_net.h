//
// Created by LvMeng on 2022/9/29 16:01.
// Note: This file is used to define petri net
//
#ifndef PETRI_NET_H
#define PETRI_NET_H
#include <iostream>
#include <vector>
#include <eigen/core>
#include <eigen/SparseCore>
class PetriNet {
  // TODO: void的函数是否可以改成BOOL函数来返回，提升程序健壮性；【定义宏变量】
private:
  //PN={P,T,I,O,Mi}
  Eigen::VectorXi place_; // 库所
  Eigen::VectorXi transition_;// 变迁
  unsigned int num_of_place_ = 0;
  unsigned int num_of_trans_ = 0;
  Eigen::SparseMatrix<int> input_matrix_;// 前置矩阵
  Eigen::SparseMatrix<int> output_matrix_;//后置矩阵
  Eigen::SparseMatrix<int> trans_matrix_;
  std::vector<int> firable_transition_;//可激发的变迁保存到成员变量中
  Eigen::VectorXi marking_;// 状态标识
  static PetriNet *instance_;  //单例对象指针

  // 构造和析构成为私有的, 禁止外部构造和析构
  PetriNet(Eigen::VectorXi &p, Eigen::VectorXi &t,
           Eigen::MatrixXi &i, Eigen::MatrixXi &o,
           Eigen::VectorXi &m_0);
  PetriNet() = default;
  ~PetriNet() = default;
  // 更新可用激发
  void FreshFirableTransition();
public:
  // 禁止外部拷贝和赋值
  PetriNet(const PetriNet &) = delete;
  const PetriNet &operator=(const PetriNet &) = delete;
  // 实例化创建。获得本类实例的唯一全局访问点
  static PetriNet *GetInstance(Eigen::VectorXi &p, Eigen::VectorXi &t,
                               Eigen::MatrixXi &i, Eigen::MatrixXi &o,
                               Eigen::VectorXi &m_0);
  // 获取当前petri_net的Marking
  const Eigen::VectorXi &GetMarking() const;
  // 设置marking
  void SetMarking(const Eigen::VectorXi &marking);
  // 获取可用的激发
  const std::vector<int> &GetFirableTransition() const;
  // 激发一个transition，并改变petri net的状态
  void FiringATransition(int t);
};
#endif //PETRI_NET_H
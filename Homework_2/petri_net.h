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
#include "./time_axis.h"
#define PETRI_NET_SUCCESS true
#define PETRI_NET_FAIL false
/* PetriNet基类:
 * 1. 继承自PetriNet;
 * 2. PN={P,T,I,O,Mi};
 * 3. 采取从外部fire的策略，激发前应当检查可激发的变迁;
 * 4. 对外提供接口:
 *      GetMarking - 获取petri网的状态;
 *      SetMarking - 设置petri网的状态;
 *      GetFirableTransition - 获取可激发的变迁;
 *      FiringATransition - 激发一个变迁;
 */
class PetriNet {
protected:
  Eigen::VectorXi place_; // 库所
  Eigen::VectorXi transition_;// 变迁
  unsigned int num_of_place_ = 0;
  unsigned int num_of_trans_ = 0;
  Eigen::SparseMatrix<int> input_matrix_;// 前置矩阵
  Eigen::SparseMatrix<int> output_matrix_;//后置矩阵
  Eigen::SparseMatrix<int> trans_matrix_;
  std::vector<int> firable_transition_;//可激发的变迁
  Eigen::VectorXi marking_;// 状态标识
  // 更新可用激发
  bool FreshFirableTransition();
public:
  PetriNet() = default;
  ~PetriNet() = default;
  PetriNet(Eigen::VectorXi &p, Eigen::VectorXi &t,
           Eigen::MatrixXi &i, Eigen::MatrixXi &o,
           Eigen::VectorXi &m_0);
  // 获取当前petri_net的Marking
  const Eigen::VectorXi &GetMarking() const;
  // 设置marking
  void SetMarking(const Eigen::VectorXi &marking);
  // 获取可用的激发
  const std::vector<int> &GetFirableTransition() const;
  // 激发一个transition，并改变petri net的状态
  bool FiringATransition(int t);
};

  /* 库所附时网PlaceTimedPetriNet:
   * 1. PetriNet的派生类;
   * 2. 加入时间轴Time_Axis;
   * 3. 采取从外部fire的策略，激发前需要检查可激发的变迁，需要更新;
   */
class PlaceTimedPetriNet:public PetriNet{
private:
  TimeAxis time_axis_;// 时间轴
  Eigen::VectorXi place_delay_;// 库所时延（客观配置，petri网生成就写死了）
  Eigen::VectorXi place_state_;// 库所是否冷却完成
public:
  PlaceTimedPetriNet() = default;
};
  /* 变迁附时petri网TransitionTimedPetriNet
  * 1. PetriNet的派生类;
  * 2. 加入时间轴Time_Axis;
  * 3. 采取从外部fire的策略，激发前需要检查可激发的变迁，需要更新;
  */
class TransitionTimedPetriNet:public PetriNet{

};
#endif //PETRI_NET_H
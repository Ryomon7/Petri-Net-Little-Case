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
#define PETRI_NET_SUCCESS true
#define PETRI_NET_FAIL false
#define MAX_INT (((unsigned int)(-1))>>1)
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
  unsigned int num_of_place_ = 0;// 库所数
  unsigned int num_of_trans_ = 0;// 变迁数
  Eigen::SparseMatrix<int> input_matrix_;// 前置矩阵
  Eigen::SparseMatrix<int> output_matrix_;//后置矩阵
  Eigen::SparseMatrix<int> trans_matrix_;
  std::vector<int> firable_transition_;//可激发的变迁
  Eigen::VectorXi marking_;// 状态标识
private:
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
 * 1. PetriNet的派生类, PN={P,T,I,O,Mi,D};
 * 2. 采取从外部fire的策略，激发前需要检查可激发的变迁，需要更新;
 * 3. 掩盖基类FreshFirableTransition、GetFirableTransitio、FiringATransition
 */
class PlaceTimedPetriNet : public PetriNet {
private:
  int time_origin_;// 时间起点
  int time_stamp_;// 时间戳--g
  std::pair<int,int> next_firable_information_;// 下一个变迁准备就绪还需要等待的 <时间, trans> --\lambda
  Eigen::VectorXi place_delay_;// 库所需要时延（客观配置）--d
  Eigen::VectorXi token_entry_time_;// token入库时间, 激发后需要修改入库时间
  Eigen::VectorXi token_wait_time_;// 等待时间--v, 更新时间戳后需要修改等待时间
  std::vector<int> untimed_firable_transition_;// 在未来可能激发的变迁

  bool UntimedFreshFirableTransition();// 刷新TimedPetriNet <未来> 可激发的变迁
  bool CalcNextFirableInformation();// 计算下次激发需等待的最短时间
  bool FreshFirableTransition();// 刷新TimedPetriNet <当前> 可激发的变迁
  bool FreshTokenWaitTime();// 刷新Token等待时间
  bool FreshPlaceTimedPetriNet();// 集合了刷新操作
public:
  PlaceTimedPetriNet(Eigen::VectorXi &p,
                     Eigen::VectorXi &t,
                     Eigen::MatrixXi &i,
                     Eigen::MatrixXi &o,
                     Eigen::VectorXi &m_0,
                     Eigen::VectorXi &place_delay) : PetriNet(p, t, i, o, m_0),
                                                     place_delay_(place_delay),
                                                     time_origin_(0),
                                                     time_stamp_(0) {
      token_entry_time_= Eigen::VectorXi::Zero(num_of_place_);
      token_wait_time_= Eigen::VectorXi::Zero(num_of_place_);
      FreshPlaceTimedPetriNet();
  }
  // 设置时间戳
  void SetTimeStamp(int time_stamp);
  int GetTimeStamp() const;
  // 获取Token等待时间--v
  const Eigen::VectorXi &GetTokenWaitTime() const;
  /********************************************
   * （遮盖）激发一个transition，并改变petri net的状态
   *  激发之后记得去修改，进库时间，petri网的时间戳
   ********************************************/
  bool FiringATransition(int t, int time_stamp);
  // 获取下次变换信息
  const std::pair<int, int> &GetNextFirableInformation() const;

};
/* 变迁附时petri网TransitionTimedPetriNet */
class TransitionTimedPetriNet : public PetriNet {};
#endif //PETRI_NET_H
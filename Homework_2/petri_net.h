//
// Created by LvMeng on 2022/9/29 16:01.
// Note: This file is used to define petri net
//
#ifndef PETRI_NET_H
#define PETRI_NET_H
#include <iostream>
#include <utility>
#include <vector>
#include <eigen/core>
#include <eigen/SparseCore>
#define PETRI_NET_SUCCESS true
#define PETRI_NET_FAIL false
#define MAX_INT (((unsigned int)(-1))>>1)
/* **************************************
 * PetriNet基类:
 * 1. 继承自PetriNet;
 * 2. PN={P,T,I,O,Mi};
 * 3. 采取从外部fire的策略，激发前应当检查可激发的变迁;
 * 4. 对外提供接口:
 *      GetMarking - 获取petri网的状态;
 *      SetMarking - 设置petri网的状态;
 *      GetFirableTransition - 获取可激发的变迁;
 *      FiringATransition - 激发一个变迁;
 * **************************************/
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
  bool FreshFirableTransition();
public:
  PetriNet() = default;
  ~PetriNet() = default;
  PetriNet(Eigen::VectorXi &p, Eigen::VectorXi &t,
           Eigen::MatrixXi &i, Eigen::MatrixXi &o,
           Eigen::VectorXi &m_0) {
      place_ = p;
      transition_ = t;
      num_of_place_ = static_cast<int>(p.size());
      num_of_trans_ = static_cast<int>(t.size());
      // 采用稀疏矩阵来表示input、output、trans，优点：节省存储空间
      input_matrix_ = i.sparseView();
      output_matrix_ = o.sparseView();
      trans_matrix_ = (o - i).sparseView();
      // 初始状态就是m0
      marking_ = m_0;
      FreshFirableTransition();
  }
  const Eigen::VectorXi &GetMarking() const;
  void SetMarking(const Eigen::VectorXi &marking);
  const std::vector<int> &GetFirableTransition() const;
  bool FiringATransition(int t);
};

/* **************************************
 * Timed_Petri_Net信息类:
 * 保存TimedPetriNet的状态信息, 方便reset时使用使用
 * **************************************/
class TimedPetriNetInfo {
public:
  int time_stamp_info = 0;// 时间戳--g
  Eigen::VectorXi marking_info; // 状态--M
  Eigen::VectorXi token_wait_time_info; // token在库等待时间信息--v
  Eigen::VectorXi token_entry_time_info;// token入库时间信息

  TimedPetriNetInfo() = default;
  TimedPetriNetInfo(int time_stamp_info,
                    Eigen::VectorXi marking_info,
                    Eigen::VectorXi token_wait_time_info,
                    Eigen::VectorXi token_entry_time_info)
      : time_stamp_info(time_stamp_info),
        marking_info(std::move(marking_info)),
        token_wait_time_info(std::move(token_wait_time_info)),
        token_entry_time_info(std::move(token_entry_time_info)) {}
};
/* **************************************
 * 库所附时网PlaceTimedPetriNet类:
 * 1. PetriNet的派生类, PN={P,T,I,O,Mi,D};
 * 2. 采取从外部fire的策略，激发前需要检查可激发的变迁，需要更新;
 * 3. FreshFirableTransition、FiringATransition
 * **************************************/
class PlaceTimedPetriNet : public PetriNet {
private:
  int time_origin_;// 时间起点
  int time_stamp_;// 时间戳--g
  std::pair<int, int> next_firable_information_;// 下一个变迁准备就绪还需要等待的 <时间, trans> --\lambda
  Eigen::VectorXi place_delay_;// 库所需要时延（客观配置）--d
  Eigen::VectorXi token_wait_time_;// 等待时间--v, 更新时间戳后需要修改等待时间
  Eigen::VectorXi token_entry_time_;// token入库时间, 激发后需要修改入库时间
  std::vector<int> untimed_firable_transition_;// 在未来可能激发的变迁

  void SetTimeStamp(int time_stamp);
  bool UntimedFreshFirableTransition();
  bool CalcNextFirableInformation();
  bool FreshFirableTransition();
  bool FreshTokenWaitTime();
  bool FreshPlaceTimedPetriNet();
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
      token_entry_time_ = Eigen::VectorXi::Zero(num_of_place_);
      token_wait_time_ = Eigen::VectorXi::Zero(num_of_place_);
      FreshPlaceTimedPetriNet();
  }
  int GetTimeStamp() const;
  bool FastForward(int add_time);
  const Eigen::VectorXi &GetTokenWaitTime() const;
  void SetTokenWaitTime(const Eigen::VectorXi &token_wait_time);
  const Eigen::VectorXi &GetTokenEntryTime() const;
  void SetTokenEntryTime(const Eigen::VectorXi &token_entry_time);
  bool FiringATransition(int t, int add_time_stamp);
  const std::pair<int, int> &GetNextFirableInformation() const;
  bool ResetTimedPetriNet(const TimedPetriNetInfo &timed_petri_net_info);
};
/* 变迁附时petri网TransitionTimedPetriNet */
class TransitionTimedPetriNet : public PetriNet {};
#endif //PETRI_NET_H
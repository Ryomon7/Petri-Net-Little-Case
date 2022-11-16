//
// Created by LvMeng on 2022/11/9 10:43.
// Note: This file is used to……
// Copyright (C) 2022 * Ltd. All rights reserved.
//
#ifndef TIMED_REACHABILITY_GRAPH_H_
#define TIMED_REACHABILITY_GRAPH_H_
#include <unordered_map>
#include <utility>
#include "petri_net.h"
#define REACHABILITY_GRAPH_SUCCESS true
#define REACHABILITY_GRAPH_FAIL false

namespace timed {
/* **************************************
 * Timed_Petri_Net Reachability graph的节点类:
 * 保存节点信息：
 *
 * **************************************/
class MarkingNode {
public:
  int node_name_ = -1;// 状态节点的编号（Name）
  TimedPetriNetInfo timed_petri_net_info_;// Timed PN保存的节点信息
  std::vector<std::pair<int, int>> transition_to_son_;// pair<which_transition, son_node>
  MarkingNode(int node_name, TimedPetriNetInfo timed_petri_net_info)
      : node_name_(node_name), timed_petri_net_info_(std::move(timed_petri_net_info)) {}
  MarkingNode() = default;
};
// 附时可达图类
class ReachabilityGraph {
private:
  // 维护nodes_结构体数组表示可达图
  std::vector<MarkingNode> nodes_;
protected:
  // 用于标识当前共有多少个状态
  int marking_number_ = 0;
  // 维护v_new_，保存已出现但未激发的状态，因为v_new_需遍历/增删首尾元素，用vector较合理
  std::vector<TimedPetriNetInfo> v_new_;
  // 维护v_old_，反向检索某状态是否已出现过，因v_old_只需添加/检索无需遍历，用unordered_map更快
  std::unordered_map<std::string, std::pair<bool, int>> v_old_;// pair<是否已激发过，此状态分配的编号>
  bool AddNode(const int &node_name,
               const TimedPetriNetInfo &timed_petri_net_info,
               const std::vector<std::pair<int, int>> &transition_to_son);
  bool AddNode(const int &node_name,
               const TimedPetriNetInfo &timed_petri_net_info);
  bool ChangeNodeInfo(const int &node_name, const TimedPetriNetInfo &new_info);
  // 因为不想自己写hash函数，所以多绕一步将vector转化为string
  static std::string Vector2String(Eigen::VectorXi Eigen_vector_int);
  static std::string GetStringFromNetInfo(const PlaceTimedPetriNet *timed_petri_net);
  static std::string GetStringFromNetInfo(const TimedPetriNetInfo &timed_petri_net_info);
  int GetNodeNumberInVOld(const std::string &info_str) const;
  bool GetNodeStatusInVOld(const std::string &info_str) const;
  bool SetNodeFiredInVOld(const std::string &info_str);
public:
  bool BuildReachabilityGraph(PlaceTimedPetriNet *timed_petri_net, Eigen::VectorXi goal_making);
  const std::vector<MarkingNode> &GetNodes() const;
};
}
#endif //TIMED_REACHABILITY_GRAPH_H_

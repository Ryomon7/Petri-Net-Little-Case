//
// Created by LvMeng on 2022/10/2 14:11.
// Note: This file is used to define petri net
//
#ifndef REACHABILITY_GRAPH_H_
#define REACHABILITY_GRAPH_H_
#include <unordered_map>
#include <utility>
#include "petri_net.h"
#define REACHABILITY_GRAPH_SUCCESS true
#define REACHABILITY_GRAPH_FAIL false
// 可达图 G = {V, E, W}
// 节点类
class MarkingNode {
public:
  int node_name_ = -1;// 状态节点的编号（Name）
  Eigen::VectorXi marking_;// 本节点状态, Mi = {P1,P2,P3...}
  std::vector<std::pair<int, int>> transition_to_son_;// pair<which_transition, son_node>
  MarkingNode(int node_name, Eigen::VectorXi marking);
  MarkingNode() = default;
};
// 可达图类
class ReachabilityGraph {
private:
  // 维护nodes_结构体数组表示可达图
  std::vector<MarkingNode> nodes_;
protected:
  // 用于标识当前共有多少个状态
  int marking_number_ = 0;
  // 维护v_new_，保存已出现但未激发的状态，因为v_new_需遍历/增删首尾元素，用vector较合理
  std::vector<Eigen::VectorXi> v_new_;
  // 维护v_old_，反向检索某状态是否已出现过，因v_old_只需添加/检索无需遍历，用unordered_map更快
  std::unordered_map<std::string, std::pair<bool, int>> v_old_;// pair<是否已激发过，此状态分配的编号>
  bool AddNode(const int &node_name,
               const Eigen::VectorXi &marking,
               const std::vector<std::pair<int, int>> &transition_to_son);
  bool AddNode(const int &node_name,
               const Eigen::VectorXi &marking);
  //TODO: 因为不想自己写hash函数，所以多绕一步将vector转化为string
  static std::string Vector2String(Eigen::VectorXi Eigen_vector_int);
  int GetNodeNumberInVOld(const Eigen::VectorXi &mark) const;
  bool GetNodeStatusInVOld(const Eigen::VectorXi &mark) const;
  bool SetNodeFiredInVOld(const Eigen::VectorXi &mark);
public:
  bool BuildReachabilityGraph(PetriNet *petri_net);
  // 外部调用GetNodes()获取成员变量nodes_即可以描述可达图
  const std::vector<MarkingNode> &GetNodes() const;
};
#endif //REACHABILITY_GRAPH_H_

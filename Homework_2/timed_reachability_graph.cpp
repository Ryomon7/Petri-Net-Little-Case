//
// Created by LvMeng on 2022/11/9 10:43.
// Note: This file is used to……
// Copyright (C) 2022 * Ltd. All rights reserved.
//
#include "timed_reachability_graph.h"
/** ****************************
 * @brief: 外部获取可达图
 * @param: NULL
 * @return: nodes_ 使用这个变量即可描述可达图
 * @author: RyoMon
 * @date:
 * *****************************/
const std::vector<timed::MarkingNode> &timed::ReachabilityGraph::GetNodes() const {
    return nodes_;
}
/** **************************** 
 * @brief: 向可达图中添加节点
 * @param:
 *   1. node_name: 节点编号
 *   2. timed_petri_net_info: timed petri net的信息
 *   3. transition_to_son: 指向儿子节点的信息<transition, son_node>
 * @return: NULL
 * @author: RyoMon
 * @date:
 * *****************************/
bool timed::ReachabilityGraph::AddNode(const int &node_name,
                                       const TimedPetriNetInfo &timed_petri_net_info,
                                       const std::vector<std::pair<int, int>> &transition_to_son) {
    MarkingNode node;
    node.node_name_ = node_name;
    node.timed_petri_net_info_ = timed_petri_net_info;
    node.transition_to_son_ = transition_to_son;
    nodes_.emplace_back(node);
    return REACHABILITY_GRAPH_SUCCESS;
}
bool timed::ReachabilityGraph::AddNode(const int &node_name, const TimedPetriNetInfo &timed_petri_net_info) {// 重载，两个参数
    MarkingNode node(node_name, timed_petri_net_info);
    nodes_.emplace_back(node);
    return REACHABILITY_GRAPH_SUCCESS;
}
/** ****************************
 * @brief: 修改可达图 nodes_ 中某个节点的状态
 * @param:
 *   1. node_name: 想要修改的节点编号
 *   2. timed_petri_net_info: 新的timed petri net的信息
 * @return: 修改成功输出true, 失败输出false.
 * @author: RyoMon
 * @date:
 * *****************************/
bool timed::ReachabilityGraph::ChangeNodeInfo(const int &node_name, const TimedPetriNetInfo &new_info) {
    int len = static_cast<int>(nodes_.size());
    for (int i = 0; i < len; i++) {
        if (nodes_[i].node_name_ == node_name) {
            nodes_[i].timed_petri_net_info_ = new_info;
            return REACHABILITY_GRAPH_SUCCESS;
        }
    }
    std::cout << "fail to find node" << node_name << std::endl;
    return REACHABILITY_GRAPH_FAIL;
}

/** ****************************
 * @brief: 建立可达图
 * @param:
 *   1. timed_petri_net: Timed Petri Net的指针
 *   2. goal_making: 目标状态
 * @return: 建立成功输出true, 建立失败输出false.
 * @author: RyoMon
 * @date:
 * *****************************/
bool timed::ReachabilityGraph::BuildReachabilityGraph(PlaceTimedPetriNet *timed_petri_net,
                                                      Eigen::VectorXi goal_making) {
    if (timed_petri_net == nullptr) return REACHABILITY_GRAPH_FAIL;
    v_new_.clear();
    v_old_.clear();
    marking_number_ = 0;
    // m_0添加到数据结构
    TimedPetriNetInfo status_zero(timed_petri_net->GetTimeStamp(),
                                  timed_petri_net->GetMarking(),
                                  timed_petri_net->GetTokenWaitTime(),
                                  timed_petri_net->GetTokenEntryTime());
    v_new_.emplace_back(status_zero);
    std::string zero_str = GetStringFromNetInfo(timed_petri_net);
    v_old_.emplace(zero_str, std::make_pair(false, marking_number_));// status=false: 未激发
    AddNode(marking_number_, status_zero);// 向添加M0节点
    int important_place_num = goal_making[goal_making.size() - 1];// 目标库所位置（对应产品打包的框子）
    bool is_continue_grow = true;// 无限图需要深度限制, is_continue_grow置为false不再向v_new添加新节点
    while (!v_new_.empty()) {// 目标是清空v_new
        TimedPetriNetInfo present_status = v_new_.back();// new节点的状态
        std::string present_status_str = GetStringFromNetInfo(present_status);// new节点状态（字符串）
        v_new_.pop_back();
        int present_index = GetNodeNumberInVOld(present_status_str);// new节点的编号(已为节点分配)
        timed_petri_net->ResetTimedPetriNet(present_status);// Timed petri net 设置为本状态
        std::vector<int> firable_t = timed_petri_net->GetFirableTransition();// 获得此状态下立即可以firable的t
        if (!firable_t.empty()) {// 若有可立即激发的变迁，则遍历激发当前变迁
            unsigned int length = firable_t.size();
            for (int i = 0; i < length; i++) {// 遍历当前可激发序列，激发并记录子节点状态，装入v_new_
                int transition_name = firable_t[i];
                timed_petri_net->ResetTimedPetriNet(present_status);// 复位
                timed_petri_net->FiringATransition(transition_name, 0);// fire, 都是0秒激发
                TimedPetriNetInfo follow_status(timed_petri_net->GetTimeStamp(),
                                                timed_petri_net->GetMarking(),
                                                timed_petri_net->GetTokenWaitTime(),
                                                timed_petri_net->GetTokenEntryTime());
                std::string follow_status_str = GetStringFromNetInfo(follow_status);
                int follow_index = GetNodeNumberInVOld(follow_status_str);// 获取激发后状态的编号
                if (follow_index == -1) {// 若状态没出现过会返回编号-1
                    marking_number_++;// 出现全新状态，计数器+1
                    // 若继续生长（不剪枝），将新的节点放入v_new_等待激发
                    if (is_continue_grow) { v_new_.emplace_back(follow_status); }
                    v_old_.emplace(follow_status_str,
                                   std::make_pair(false, marking_number_));// 将其加入v_old_并记录分配的编号，状态设置为未激发
                    AddNode(marking_number_, follow_status);// 可达图新增一个节点
                    follow_index = marking_number_;
                }
                // 因为是不同的激发（遍历），即使子节点状态出现过，应该也要新增弧（意思是无需校验子节点与前面是否相同）
                nodes_[present_index].transition_to_son_.emplace_back(std::make_pair(transition_name, follow_index));
            }
            SetNodeFiredInVOld(present_status_str);
        } else {// 若无可立即激发的变迁，则快进到下一个时刻
            int shortest_time = timed_petri_net->GetNextFirableInformation().first;
            timed_petri_net->FastForward(shortest_time);// 快进到最近的可激发的时间
            TimedPetriNetInfo fast_forward_status(timed_petri_net->GetTimeStamp(),
                                                  timed_petri_net->GetMarking(),
                                                  timed_petri_net->GetTokenWaitTime(),
                                                  timed_petri_net->GetTokenEntryTime());
            v_new_.emplace_back(fast_forward_status);// 时间快进后的状态重新放回修改v_new_
            // 无需修改v_old_，因为v_old_只用m做索引
            std::string fast_forward_str = GetStringFromNetInfo(fast_forward_status);
            int fast_forward_index = GetNodeNumberInVOld(fast_forward_str);// 获取快进后状态的编号
            if (fast_forward_index == -1) {// 若状态没出现过会返回编号-1
                return REACHABILITY_GRAPH_FAIL;
            }
            ChangeNodeInfo(fast_forward_index, fast_forward_status);// 修改nodes_
        }
        if (!v_new_.empty()) {
            while (v_new_.back().marking_info[v_new_.back().marking_info.size() - 1] == important_place_num) {
                v_new_.pop_back();// 如果满足goal marking条件就剪枝，
                is_continue_grow = false;//停止向v_new添加节点
            }
        }
    }
    return REACHABILITY_GRAPH_SUCCESS;
}
/** ****************************
 * @brief: Eigen vector转换为string
 * @param: 一个vectorXi类型的向量
 * @return: 转换为str的结果
 * @author: RyoMon
 * @date:
 * *****************************/
std::string timed::ReachabilityGraph::Vector2String(Eigen::VectorXi Eigen_vector_int) {
    //std::vector<int>
    std::vector<int> arg(&Eigen_vector_int[0],
                         Eigen_vector_int.data() + Eigen_vector_int.cols() * Eigen_vector_int.rows());
    std::string strVec;
    for (auto data : arg) {
        strVec += std::to_string(data) + ",";
    }
    strVec = strVec.substr(0, strVec.size() - 1);
    return strVec;
}
/** ****************************
 * @brief: 从TimedPetriNetInfo中提取出{m, v, g}信息转换为string返回
 * @param: TimedPetriNetInfo类指针
 * @return: string 提取的字符串信息
 * @author: RyoMon
 * @date:
 * *****************************/
std::string timed::ReachabilityGraph::GetStringFromNetInfo(const PlaceTimedPetriNet *timed_petri_net) {
    std::string result;
    // 如果用m+v+g做键索引不太方便，因为涉及当前无可用激发，需要跳转到下一个时刻，但两个时刻的v，g显然不一样了，进行hash索引是不方便的
    result = Vector2String(timed_petri_net->GetMarking());// m
    return result;
}
std::string timed::ReachabilityGraph::GetStringFromNetInfo(const TimedPetriNetInfo &timed_petri_net_info) {
    std::string result;
    // 如果用m+v+g做键索引不太方便因为涉及当前无可用激发，需要跳转到下一个时刻，但两个时刻的v，g显然不一样了，进行hash索引是不方便的
    result = Vector2String(timed_petri_net_info.marking_info);// m
    return result;
}
/** ****************************
 * @brief: 查询V_Old_为某个状态分配的编号
 * @param: info_str: 用于索引哈希表的键值对
 * @return: 为状态分配的编号(Node编号)，若状态不存在于hash表则返回-1
 * @author: RyoMon
 * @date:
 * *****************************/
int timed::ReachabilityGraph::GetNodeNumberInVOld(const std::string &info_str) const {
    auto iter = v_old_.find(info_str);
    if (iter == v_old_.end()) {
        return -1;
    }
    return iter->second.second;// 编号
}
/** ****************************
 * @brief: 查询V_Old_某个状态是否已遍历激发
 * @param: info_str: 用于索引哈希表的键值对
 * @return: 若已经遍历激发，则返回ture，否则返回false
 * @author: RyoMon
 * @date:
 * *****************************/
bool timed::ReachabilityGraph::GetNodeStatusInVOld(const std::string &info_str) const {
    auto iter = v_old_.find(info_str);
    if (iter == v_old_.end()) {
        return REACHABILITY_GRAPH_FAIL;
    }
    return iter->second.first;// 激发状态
}
/** ****************************
 * @brief: 修改V_Old_中某状态为激发
 * @param: info_str: 用于索引哈希表的键值对
 * @return: 设置成功返回true, 设置失败返回false
 * @author: RyoMon
 * @date:
 * *****************************/
bool timed::ReachabilityGraph::SetNodeFiredInVOld(const std::string &info_str) {
    auto iter = v_old_.find(info_str);
    if (iter == v_old_.end()) {
        return REACHABILITY_GRAPH_FAIL;
    }
    iter->second.first = true;
    return REACHABILITY_GRAPH_SUCCESS;
}



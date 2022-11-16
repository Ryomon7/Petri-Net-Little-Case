//
// Created by LvMeng on 2022/10/2 16:14.
// Note: This file is used to write implementations
//
#include "reachability_graph.h"

MarkingNode::MarkingNode(int node_name, Eigen::VectorXi marking) : node_name_(node_name), marking_(std::move(marking)) {
}
/** ****************************
 * @brief: 外部获取可达图
 * @param: NULL
 * @return: 描述可达图的数据结构nodes_
 * @author: RyoMon
 * @date:
 * *****************************/
const std::vector<MarkingNode> &ReachabilityGraph::GetNodes() const {
    return nodes_;
}
/** ****************************
 * @brief: 向可达图中添加节点
 * @param:
 *  1. node_name：节点编号
 *  2. marking：节点状态
 *  3. transition_to_son：pair<激发某变迁, 可以到达的节点>
 * @return: 成功返回true，失败返回false
 * @author: RyoMon
 * @date:
 * *****************************/
bool ReachabilityGraph::AddNode(const int &node_name,
                                const Eigen::VectorXi &marking,
                                const std::vector<std::pair<int, int>> &transition_to_son) {
    MarkingNode node;
    node.node_name_ = node_name;
    node.marking_ = marking;
    node.transition_to_son_ = transition_to_son;
    nodes_.emplace_back(node);
    return REACHABILITY_GRAPH_SUCCESS;
}
bool ReachabilityGraph::AddNode(const int &node_name, const Eigen::VectorXi &marking) {// 重载，两个参数
    MarkingNode node(node_name, marking);
    nodes_.emplace_back(node);
    return REACHABILITY_GRAPH_SUCCESS;
}
/** ****************************
 * @brief: 建立可达图
 * @param: petri_net 一个指向petri net对象的指针
 * @return: 成功返回true，失败返回false
 * @author: RyoMon
 * @date:
 * *****************************/
bool ReachabilityGraph::BuildReachabilityGraph(PetriNet *petri_net) {
    if (petri_net == nullptr) return REACHABILITY_GRAPH_FAIL;
    v_new_.clear();
    v_old_.clear();
    marking_number_ = 0;
    //添加m_0
    Eigen::VectorXi marking_zero = petri_net->GetMarking();
    v_new_.emplace_back(marking_zero);
    v_old_.emplace(Vector2String(marking_zero), std::make_pair(false, marking_number_));// status=false: 未激发
    AddNode(marking_number_, marking_zero);// 添加M0节点
    while (!v_new_.empty()) {// 清空v_new
        Eigen::VectorXi present_marking = v_new_.back();// 本节点的状态
        v_new_.pop_back();
        int present_index = GetNodeNumberInVOld(present_marking);// 本节点的编号
        petri_net->SetMarking(present_marking);
        std::vector<int> firable_t = petri_net->GetFirableTransition();// 获得此状态下firable的t
        unsigned int length = firable_t.size();
        for (int i = 0; i < length; i++) {// 遍历激发
            int transition_name = firable_t[i];
            petri_net->SetMarking(present_marking);// 复位
            petri_net->FiringATransition(transition_name);// fire
            Eigen::VectorXi follow_marking = petri_net->GetMarking();
            int follow_index = GetNodeNumberInVOld(follow_marking);// 校验激发后状态
            if (follow_index == -1) {// 若marking没出现过会返回-1
                marking_number_++;// 出现全新状态，计数器+1
                v_new_.emplace_back(follow_marking); // 加入v_new_等待激发
                v_old_.emplace(Vector2String(follow_marking),
                               std::make_pair(false, marking_number_));// 加入v_old_并记录分配的编号，状态设置为未激发
                AddNode(marking_number_, follow_marking);// 可达图新增节点
                follow_index = marking_number_;
            }
            // 因为是不同的激发（遍历），即使子节点状态出现过，也要新增弧（意思是无需校验子节点与前面是否相同）
            nodes_[present_index].transition_to_son_.emplace_back(std::make_pair(transition_name, follow_index));
        }
        SetNodeFiredInVOld(present_marking);
    }
    return REACHABILITY_GRAPH_SUCCESS;
}
/** ****************************
 * @brief: 将Eigen vector转换为string
 * @param: Eigen vector
 * @return: strVec：字符串格式
 * @author: RyoMon
 * @date:
 * *****************************/
std::string ReachabilityGraph::Vector2String(Eigen::VectorXi Eigen_vector_int) {
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
 * @brief: 查询V_Old_中为某个状态分配的编号
 * @param: mark为要查询的状态
 * @return: 返回输入状态对应的编号，如果并未记录此状态则返回-1
 * @author: RyoMon
 * @date:
 * *****************************/
int ReachabilityGraph::GetNodeNumberInVOld(const Eigen::VectorXi &mark) const {
    std::string mark_str = Vector2String(mark);
    auto iter = v_old_.find(mark_str);
    if (iter == v_old_.end()) {
        return -1;
    }
    return iter->second.second;// 编号
}
/** ****************************
 * @brief: 查询V_Old_某个状态是否已遍历激发
 * @param: mark为要查询的状态
 * @return: 返回true表示已经遍历激发，否则返回false
 * @author: RyoMon
 * @date:
 * *****************************/
bool ReachabilityGraph::GetNodeStatusInVOld(const Eigen::VectorXi &mark) const {
    std::string mark_str = Vector2String(mark);
    auto iter = v_old_.find(mark_str);
    if (iter == v_old_.end()) {
        return REACHABILITY_GRAPH_FAIL;
    }
    return iter->second.first;// 激发状态
}
/** ****************************
 * @brief: 修改V_Old_中某状态为已激发
 * @param: mark为要查询的状态
 * @return: 成功返回true，否则返回false
 * @author: RyoMon
 * @date:
 * *****************************/
bool ReachabilityGraph::SetNodeFiredInVOld(const Eigen::VectorXi &mark) {
    std::string mark_str = Vector2String(mark);
    auto iter = v_old_.find(mark_str);
    if (iter == v_old_.end()) {
        return REACHABILITY_GRAPH_FAIL;
    }
    iter->second.first = true;
    return REACHABILITY_GRAPH_SUCCESS;
}






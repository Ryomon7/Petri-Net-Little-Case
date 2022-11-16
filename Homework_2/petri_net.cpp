//
// Created by LvMeng on 2022/9/29 20:53.
// Note: This file is used to write implementations
//
#include <vector>
#include "petri_net.h"
/** ****************************
 * @brief: 获取当前petri_net的Marking
 * @param: NULL
 * @return: Petri网成员变量 marking
 * @author: RyoMon
 * @date:
 * *****************************/
const Eigen::VectorXi &PetriNet::GetMarking() const {
    return marking_;
}
/** ****************************
 * @brief: 设置marking
 * @param: marking为要设置的状态
 * @return: NULL
 * @author: RyoMon
 * @date:
 * *****************************/
void PetriNet::SetMarking(const Eigen::VectorXi &marking) {
    marking_ = marking;
    FreshFirableTransition();
}
/** ****************************
 * @brief: 获取可用激发
 * @param: NULL
 * @return: 成员函数 firable_transition_
 * @author: RyoMon
 * @date:
 * *****************************/
const std::vector<int> &PetriNet::GetFirableTransition() const {
    return firable_transition_;
}
/** ****************************
 * @brief: 更新当前可用激发(marking改变后皆需调用此函数)
 * @param: NULL
 * @return: 更新成功返回true, 失败则返回false
 * @author: RyoMon
 * @date:
 * *****************************/
bool PetriNet::FreshFirableTransition() {
    Eigen::VectorXi vector_temp(num_of_place_);
    std::vector<int> firable_trans;
    for (int i = 0; i < num_of_trans_; i++) {
        vector_temp = marking_ - input_matrix_.col(i);
        bool is_firable = true;
        //TODO:此处可优化，改用稀疏矩阵判断是否可激发
        for (auto &j : vector_temp) {
            is_firable = (j >= 0) & is_firable;
        }
        if (is_firable) firable_trans.emplace_back(i);
    }
    firable_transition_ = firable_trans;
    return PETRI_NET_SUCCESS;
}
/** ****************************
 * @brief: 激发一个transition，会改变petri net的marking
 * @param: t为要激发的变迁的编号
 * @return: 激发成功返回true, 失败则返回false
 * @author: RyoMon
 * @date:
 * *****************************/
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
/***********************************************************/
/** ****************************
 * @brief: 获取Token在库所的等待时间
 * @param: NULL
 * @return: 返回成员变量token_wait_time_
 * @author: RyoMon
 * @date:
 * *****************************/
const Eigen::VectorXi &PlaceTimedPetriNet::GetTokenWaitTime() const {
    return token_wait_time_;
}
/** ****************************
 * @brief: 设置Token在库所的等待时间(用于重置petri网)
 * @param: token_wait_time为要设置的等待时间
 * @return: NULL
 * @author: RyoMon
 * @date:
 * *****************************/
void PlaceTimedPetriNet::SetTokenWaitTime(const Eigen::VectorXi &token_wait_time) {
    token_wait_time_ = token_wait_time;
}
/** ****************************
 * @brief: 刷新Timed_Petri_Net当前可立即激发的变迁
 * @param: NULL
 * @return: 刷新成功返回true，失败则返回false
 * @author: RyoMon
 * @date:
 * *****************************/
bool PlaceTimedPetriNet::FreshFirableTransition() {
    // 从untimed_firable_transition_中，考虑延时是否足够。
    Eigen::VectorXi delay_available(num_of_place_); // delay时间需要足够
    delay_available = token_wait_time_ - place_delay_;
    std::vector<int> firable_trans;
    for (auto i : untimed_firable_transition_) {
        Eigen::VectorXi trans_need_token = input_matrix_.col(i);
        bool is_delay_enough = true;
        for (int j = 0; j < num_of_place_; j++) {
            // 只有这个place的token在trans中被需要时，才去看它的delay是否满足要求
            if (trans_need_token[j] > 0) is_delay_enough &= (delay_available[j] >= 0);
        }
        if (is_delay_enough) firable_trans.emplace_back(i);
    }
    firable_transition_ = firable_trans;// 当前可激发的变迁
    return PETRI_NET_SUCCESS;
}
/** ****************************
 * @brief: 计算下次激发所需要的最少等待时间, 即刷新 pair<time, trans>
 * @param: NULL
 * @return: 计算成功返回true，失败则返回false
 * @author: RyoMon
 * @date:
 * *****************************/
bool PlaceTimedPetriNet::CalcNextFirableInformation() {
    // 写此函数应该是以Untimed_Firable_Transition为导向, 因此调用此函数以前需要先更新untimed_trans
    // 看每一个T的Place还需要等待多长时间，每个T在未来都是可以激发的！
    Eigen::VectorXi duration(num_of_place_);
    duration = token_wait_time_ - place_delay_;
    if (untimed_firable_transition_.empty()) {
        std::cout << "Ryomon Untimed firable transition empty" << std::endl;
        return PETRI_NET_FAIL;
    }
    if (!firable_transition_.empty()) {
        // 找到当前可激发的trans中需要时间最短的，写入next_fire
        auto i = firable_transition_[0];// TODO: 如果同时有多个<当前>可激发的变迁，可以写在这里修改先激发谁的逻辑。目前是先激发序号小的
        next_firable_information_ = std::make_pair(0, i);// 时间写0, “当前".
        return PETRI_NET_SUCCESS;
    }
    std::pair<int, int> next_fire_pair = std::make_pair(MAX_INT, -1);
    for (auto t : untimed_firable_transition_) {// 在成员变量”未来可激发的变迁“中选择
        // 对于所有未来可激发的trans选一个最小的time(最近)，对于某一个trans选一个最大的time
        std::pair<int, int> temp;// 一个临时pair
        temp.second = t;
        int time = 0;
        Eigen::VectorXi vector_temp = input_matrix_.col(t);
        for (int i = 0; i < num_of_place_; i++) {
            if ((vector_temp[i] > 0) & (duration[i] < 0)) {
                time = (time > (-duration[i])) ? (time) : ((-duration[i]));
            }
        }
        temp.first = time;
        next_fire_pair = (next_fire_pair.first < temp.first) ? (next_fire_pair) : (temp);
    }
    next_firable_information_ = next_fire_pair;
    return PETRI_NET_SUCCESS;
}
/** ****************************
 * @brief: 获取最近的可激发的信息
 * @param: NULL
 * @return: 下一个变迁准备就绪还需要等待的 <时间, trans> --\lambda
 * @author: RyoMon
 * @date:
 * *****************************/
const std::pair<int, int> &PlaceTimedPetriNet::GetNextFirableInformation() const {
    return next_firable_information_;
}
/** ****************************
 * @brief: 不考虑时间时，刷新TimedPetriNet <未来> 可激发的变迁
 * @param: NULL
 * @return: 成功返回true，失败则返回false
 * @author: RyoMon
 * @date:
 * *****************************/
bool PlaceTimedPetriNet::UntimedFreshFirableTransition() {
    Eigen::VectorXi vector_temp(num_of_place_);
    std::vector<int> firable_trans;
    for (int i = 0; i < num_of_trans_; i++) {
        vector_temp = marking_ - input_matrix_.col(i);
        bool is_firable = true;
        for (auto &j : vector_temp) {
            is_firable = (j >= 0) & is_firable;
        }
        if (is_firable) {
            firable_trans.emplace_back(i);
        }
    }
    untimed_firable_transition_ = firable_trans;
    return PETRI_NET_SUCCESS;
}
/** ****************************
 * @brief: 获取时间戳
 * @param: NULL
 * @return: 时间戳
 * @author: RyoMon
 * @date:
 * *****************************/
int PlaceTimedPetriNet::GetTimeStamp() const {
    return time_stamp_;
}
/** ****************************
 * @brief: 设置时间戳，设置之后刷新附时petri网
 * @param: 需要设置的时间戳
 * @return: NULL
 * @author: RyoMon
 * @date:
 * *****************************/
void PlaceTimedPetriNet::SetTimeStamp(const int time_stamp) {
    // 设置时间戳以后就要更新各个状态
    time_stamp_ = time_stamp;
    FreshPlaceTimedPetriNet();
}
/** ****************************
 * @brief: 只快进时间，而不激发变迁
 * @param: 需要快进的时间
 * @return: 成功返回true失败则返回false
 * @author: RyoMon
 * @date:
 * *****************************/
bool PlaceTimedPetriNet::FastForward(const int add_time) {
    // 保护措施，如果快进的时间太多了就返回false
    if (add_time > next_firable_information_.first) {
        std::cout << "RyoMon, Fast-forward time: " << add_time <<
                  " is larger than next_firable time: " << next_firable_information_.first << std::endl;
        return PETRI_NET_FAIL;
    }
    int time_stamp = time_stamp_ + add_time;
    SetTimeStamp(time_stamp);
    return PETRI_NET_SUCCESS;
}
/** ****************************
 * @brief: 刷新wait_time等待时间--v
 * @param: NULL
 * @return: 刷新成功返回true，失败则返回false
 * @author: RyoMon
 * @date:
 * *****************************/
bool PlaceTimedPetriNet::FreshTokenWaitTime() {
    // 激发后，更新time_stamp之后需要更新token_wait_time
    for (int i = 0; i < num_of_place_; i++) {
        if ((marking_[i] > 0) && (place_delay_[i] > 0)) {// 有token的并且库所时延非零, 才去更新v, 否则为0
            token_wait_time_[i] = time_stamp_ - token_entry_time_[i];
        } else {
            token_wait_time_[i] = 0;
        }
    }
    return PETRI_NET_SUCCESS;
}
/** ****************************
 * @brief: 激发一个transition，会改变petri net的marking
 * @param:
 *   1. t表示要激发的变迁的编号
 *   2. add_time_stamp表示需要（跳过的/等待的/延长的）时间
 * @return: 激发成功返回true, 失败则返回false
 * @author: RyoMon
 * @date:
 * *****************************/
bool PlaceTimedPetriNet::FiringATransition(int t, int add_time_stamp) {
    //保护机制，防止随便传数字，需要check一下是不是可继发
    bool is_in_firable_vec = false;
    for (auto i : untimed_firable_transition_) {
        if (t == i) {
            is_in_firable_vec = true;
            break;
        }
    }
    if (is_in_firable_vec) {
        int time_stamp = time_stamp_ + add_time_stamp;
        // 如果为可激发的transition才激发petri网 -> 改变marking
        Eigen::VectorXi trans_vector = trans_matrix_.col(t);
        Eigen::VectorXi out_vector = output_matrix_.col(t);
        // marking_ = marking_ + trans_matrix_.col(t);
        // 只有fire的时候才刷新entry time
        for (int i = 0; i < num_of_place_; i++) {
            marking_[i] = marking_[i] + trans_vector[i];
            if (marking_[i] > 0) {// 变迁之后有token(可能是新的也可能是旧的，通过out_vector[i]判断)
                token_entry_time_[i] = (out_vector[i] > 0) ? time_stamp : token_entry_time_[i];// 新token打上新的入库时间，旧的就不变
            } else {
                token_entry_time_[i] = 0;// 变迁以后无token, 就置零吧。
            }
        }
        SetTimeStamp(time_stamp);// 这里面刷新了Petri Net
        return PETRI_NET_SUCCESS;
    }
    return PETRI_NET_FAIL;
}
/** ****************************
 * @brief: 集合timed petri net 所有刷新操作
 * @param: NULL
 * @return: 刷新成功返回true，失败则返回false
 * @author: RyoMon
 * @date:
 * *****************************/
bool PlaceTimedPetriNet::FreshPlaceTimedPetriNet() {
    FreshTokenWaitTime();
    /*
       * 1. 应按照Untimed - Firable - CalcNext这个顺序构造
       * 2. 先写入untimed_firable_transition_，
       *    在CalcNext的时候会使用，因为从数量出发，计算还需要多少时间
       *    在FreshFirableTransition的时候会使用，因为在数量的基础上，考虑延时是否足够
       * 3. CalcNext放在最后，如果此时有可激发的trans，就直接使用，没有才计算。所以放最后
       */
    UntimedFreshFirableTransition();
    FreshFirableTransition();
    CalcNextFirableInformation();
    return PETRI_NET_SUCCESS;
}
/** ****************************
 * @brief: 获取token入库时间 entry time
 * @param: NULL
 * @return: token_entry_time_
 * @author: RyoMon
 * @date:
 * *****************************/
const Eigen::VectorXi &PlaceTimedPetriNet::GetTokenEntryTime() const {
    return token_entry_time_;
}
/** ****************************
 * @brief: 设置token入库时间 entry time（在重置petri网的时候才需要设置）
 * @param: 需要设置的entry time
 * @return: NULL
 * @author: RyoMon
 * @date:
 * *****************************/
void PlaceTimedPetriNet::SetTokenEntryTime(const Eigen::VectorXi &token_entry_time) {
    token_entry_time_ = token_entry_time;
}
/** ****************************
 * @brief: 重新设置Timed Petri Net的状态
 * @param: TimedPetriNetInfo一个包含了TimePetriNet状态的结构体/类
 * @return: 成功返回true失败返回false
 * @author: RyoMon
 * @date:
 * *****************************/
bool PlaceTimedPetriNet::ResetTimedPetriNet(const TimedPetriNetInfo &timed_petri_net_info) {
    time_stamp_ = timed_petri_net_info.time_stamp_info;
    SetMarking(timed_petri_net_info.marking_info);
    SetTokenWaitTime(timed_petri_net_info.token_wait_time_info);
    SetTokenEntryTime(timed_petri_net_info.token_entry_time_info);
    FreshPlaceTimedPetriNet();
    return PETRI_NET_SUCCESS;
}

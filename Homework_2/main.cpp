//
// Created by Lv Meng on 2022/9/29 11:22.
//
#include <iostream>
#include "_test_petri.h"
#include "_test_timed_petri.h"
int main() {
    printf("hello world!\n");
//    test_petri::test4();
    test_timed_petri::test11();
    return 0;
}
/* ***********************************
 * 1. 没有Template模板化，全用的int类型, timed可达图和普通可达图也可以模板化
 * 2. 内部耦合严重，需进一步抽象和封装
 * 3. 没有实现从文件中读取 PN信息
 * 4. 剪枝/设置停止条件：
 *      * 使用不等式约束
 *      * 提取向量与变换矩阵trans_matrix_相乘
 *      * 添加限制库所, 正表示离开限制库所，负表示进入限制库所。
 * ***********************************/
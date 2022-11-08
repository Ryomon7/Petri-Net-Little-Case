//
// Created by Lv Meng on 2022/9/29 11:22.
//
#include <iostream>
#include "_test_petri.h"
#include "_test_timed_petri.h"
int main() {
    printf("hello world!\n");
//    test_petri::test4();
    test_timed_petri::test7();
    return 0;
}
//  缺点一：没有模板化，全用的int类型
//  缺点二：内部耦合，需要进一步抽象和封装
//  TODO：实现从文件中读取 PN={P,T,I,O,Mi}信息



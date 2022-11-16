cmake_minimum_required(VERSION 3.23)
project(Homework_2)
include_directories(../eigen-3.4.0)
set(CMAKE_CXX_STANDARD 14)

add_executable(Homework_2 main.cpp main.cpp main.cpp petri_net.h petri_net.cpp
               reachability_graph.h reachability_graph.cpp
               tool/timer.h tool/thread_pool.h tool/map_tool.h tool/time_axis.h _test_petri.h _test_timed_petri.h timed_reachability_graph.cpp timed_reachability_graph.h)

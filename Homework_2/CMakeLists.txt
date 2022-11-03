cmake_minimum_required(VERSION 3.23)
project(Homework_2)
include_directories(../eigen-3.4.0)
set(CMAKE_CXX_STANDARD 14)

add_executable(Homework_1 main.cpp main.cpp main.cpp petri_net.h petri_net.cpp reachability_graph.h reachability_graph.cpp)

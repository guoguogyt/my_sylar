cmake_minimum_required(VERSION 2.8)

project(learnning_sylar_program)

include_directories(leileilei)

add_compile_options(-Wall -std=c++11 -O2)

add_executable(test_log ./tests/test_log.cpp ./leileilei/log.cpp)
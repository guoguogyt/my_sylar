cmake_minimum_required(VERSION 2.8)

project(learnning_sylar_program)

include_directories(leileilei)

add_compile_options(-Wall -std=c++11 -O2)

find_package(Boost REQUIRED)       
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

link_directories(/usr/local/lib64)
include_directories(/usr/local/include/yaml-cpp)

add_executable(test_log ./tests/test_log.cpp ./leileilei/log.cpp ./leileilei/config.cpp)
target_link_libraries(test_log yaml-cpp)


add_executable(test_config ./tests/test_config.cpp ./leileilei/log.cpp ./leileilei/config.cpp)
target_link_libraries(test_config yaml-cpp)
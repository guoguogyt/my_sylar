cmake_minimum_required(VERSION 2.8)

project(learnning_sylar_program)

include_directories(leileilei)

# add_compile_options(-g -Wall -std=c++11 -O2)
add_compile_options(-g -std=c++11 -O2)

find_package(Boost REQUIRED)       
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

set(LIB_SRC
    leileilei/address.cpp
    leileilei/config.cpp
    leileilei/fd_manager.cpp
    leileilei/fiber.cpp
    leileilei/hook.cpp
    leileilei/iomanager.cpp
    leileilei/log.cpp
    leileilei/mutex.cpp
    leileilei/scheduler.cpp
    leileilei/thread.cpp
    leileilei/timer.cpp
    leileilei/util.cpp
)

# 生成一个名为leileilei的动态库
add_library(leileilei SHARED ${LIB_SRC})

find_library(PTHREAD pthread)
find_library(YAMLCPP yaml-cpp)

set(LIBS
        dl
        leileilei
        pthread
        yaml-cpp
)

link_directories(/usr/local/lib64)
include_directories(/usr/local/include/yaml-cpp)

# add_executable(test_log ./tests/test_log.cpp ./leileilei/log.cpp ./leileilei/config.cpp)
# target_link_libraries(test_log yaml-cpp)

# 生成可执行的文件
add_executable(test_log ./tests/test_log.cpp)
# 提醒编译器需要先生成test_log的依赖(leileilei),然后再去生成test_log
add_dependencies(test_log leileilei)
# 指定需要连接的库
target_link_libraries(test_log ${LIBS})


# add_executable(test_config ./tests/test_config.cpp ./leileilei/log.cpp ./leileilei/config.cpp)
# target_link_libraries(test_config yaml-cpp)
add_executable(test_config ./tests/test_config.cpp)
add_dependencies(test_config leileilei)
target_link_libraries(test_config ${LIBS})


add_executable(test_thread ./tests/test_thread.cpp)
add_dependencies(test_thread leileilei)
target_link_libraries(test_thread ${LIBS})

add_executable(test_util ./tests/test_util.cpp)
add_dependencies(test_util leileilei)
target_link_libraries(test_util ${LIBS})

add_executable(test_fiber ./tests/test_fiber.cpp)
add_dependencies(test_fiber leileilei)
target_link_libraries(test_fiber ${LIBS})

add_executable(test_scheduler ./tests/test_scheduler.cpp)
add_dependencies(test_scheduler leileilei)
target_link_libraries(test_scheduler ${LIBS})

add_executable(test_iomanager ./tests/test_iomanager.cpp)
add_dependencies(test_iomanager leileilei)
target_link_libraries(test_iomanager ${LIBS})

add_executable(test_hook ./tests/test_hook.cpp)
add_dependencies(test_hook leileilei)
target_link_libraries(test_hook ${LIBS})

add_executable(test_address ./tests/test_address.cpp)
add_dependencies(test_address leileilei)
target_link_libraries(test_address ${LIBS})
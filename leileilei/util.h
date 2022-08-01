#pragma once

#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace leileilei
{

/**
 * @brief Get the Thread Id object
 *  获取线程id
 * @return pid_t 
 */
pid_t GetThreadId();



}
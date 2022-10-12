/*
 * @Descripttion: 
 * @version: 
 * @Author: leileilei
 * @Date: 2022-08-01 11:13:59
 * @LastEditors: sueRimn
 * @LastEditTime: 2022-10-12 11:33:14
 */
#pragma once

#include <cxxabi.h>
#include <execinfo.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/time.h>
#include "log.h"

namespace leileilei
{

/**
 * @brief Get the Thread Id object
 *  获取线程id
 * @return pid_t 
 */
pid_t GetThreadId();

/**
 * @brief 获取当前的调用栈
 * 
 * @param bt 存储栈
 * @param size 栈的最大层数
 * @param skip 栈跳过层数
 */
void Backtrace(std::vector<std::string>& bt, int size = 64, int skip = 1);

/**
 * @brief 获取当前的调用栈，并以字符串形式返回
 * 
 * @param size 栈的最大层数
 * @param skip 栈跳过层数
 * @param prefix 每层之前的前缀
 * @return std::string 
 */
std::string BacktraceToString(int size = 64, int skip = 2, const std::string& prefix = " ");

/**
 * @brief Get the Current M S object
 * 返回系统当前时间(毫秒)
 * @return uint64_t 
 */
uint64_t GetCurrentMS();

/**
 * @brief Get the Current U S object
 * 返回系统当前时间(微妙)
 * @return uint64_t 
 */
uint64_t GetCurrentUS();

}
/*
 * @Descripttion: 
 * @version: 
 * @Author: leileilei
 * @Date: 2022-08-01 11:13:59
 * @LastEditors: sueRimn
 * @LastEditTime: 2023-01-05 10:50:14
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


/**
 * @brief int to string
 * 
 */
std::string IntToString(int value);
/**
 * @brief string to int
 * 
 */
int StringToInt(std::string s);

/**
 * @brief float to string
 * 
 */
std::string FloatToString(float value);
/**
 * @brief string to float 
 * 
 */
float StringToFloat(std::string s);

/**
 * @brief double to string
 * 
 */
std::string DoubleToString(double value);
/**
 * @brief string to double
 * 
 */
double StringToDouble(std::string s);

/**
 * @brief 将double截断 
 * @param value double值
 * @param format 小数点后保留几位
 * @return double 
 */
double FormatFouble(double value, int format);

/**
 * @brief 将输入的字符串按照指定的字符进行分割
 * @param vec 存储分割之后的字符串
 * @param s 要被分割的字符串
 * @param spot 分隔符
 * @return true 
 * @return false 
 */
bool SplitString(std::vector<std::string>& vec, std::string s, std::string pattern = " ");

/**
 * @brief 去除字符串左端的空格
 * @param s 
 */
void TrimLeft(std::string& s);

/**
 * @brief 去除字符串右端的空格
 * @param s 
 */
void TrimRight(std::string& s);

/**
 * @brief 去除字符串两端的空格
 * @param s 
 */
void TrimAll(std::string& s);

}
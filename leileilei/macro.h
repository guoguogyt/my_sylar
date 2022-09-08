/**
 * @file macro.h
 * @author your name (you@domain.com)
 * @brief 宏是全局的    在预编译时进行直接替换
 * @version 0.1
 * @date 2022-09-07
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#pragma once

#include <string.h>
#include <assert.h>
#include "log.h"
#include "util.h"

/**
 *   long __builtin_expect(long exp, long c)函数是GCC的一个内建函数(build-in function)
    exp：
　　　　exp 为一个整型表达式, 例如: (ptr != NULL)
    c：
　　　　 c 必须是一个编译期常量, 不能使用变量
    返回值：
        返回值等于 第一个参数 exp

现在 处理器 都是 流水线 的，系统可以 提前取多条指令 进行并行处理，但遇到跳转时，则需要重新取指令，跳转指令打乱了CPU流水线。
因此，跳转次数少的程序拥有更高的执行效率。
在C语言编程时，会不可避免地使用if-else分支语句，if else 句型编译后, 一个分支的汇编代码紧随前面的代码，而另一个分支的汇编代码需要使用JMP指令才能访问到。
很明显通过JMP访问需要更多的时间, 在复杂的程序中,有很多的if else句型,又或者是一个有if else句型的库函数,每秒钟被调用几万次，通常程序员在分支预测方面做得很糟糕,
 编译器又不能精准的预测每一个分支,这时JMP产生的时间浪费就会很大。
因此，引入__builtin_expect函数来增加条件分支预测的准确性，cpu 会提前装载后面的指令，遇到条件转移指令时会提前预测并装载某个分支的指令。
编译器会产生相应的代码来优化 cpu 执行效率。
 */
#if defined __GNUC__ || defined __llvm__
#   define LEILEILEI_LIKELY(x)      __builtin_expect(!!(x), 1)
#   define LEILEILEI_UNLIKELY(x)    __builtin_expect(!!(x), 0)
#else
#   define  LEILEILEI_LIKELY(x)     (x)
#   define  LEILEILEI_UNLIKELY(x)   (x)
#endif

// 断言宏
#define LEILEILEI_ASSERT(x) \
    if(LEILEILEI_UNLIKELY(!(x)))    \
    {   \
        LEI_LOG_ERROR(LEI_LOG_GETROOTOR()) << "ASSERTION: " #x  \
            << "\nbacktrace:\n" \
            << leileilei::BacktraceToString(100, 2, "   "); \
        assert(x);  \
    }

// 断言宏
#define LEILEILEI_ASSERT2(x, w) \
    if(LEILEILEI_UNLIKELY(!(x)))    \
    {   \
        LEI_LOG_ERROR(LEI_LOG_GETROOTOR()) << "ASSERTION: " #x  \
            << "\n" << w    \
            << "\nbacktrace:\n" \
            << leileilei::BacktraceToString(100, 2, "   "); \
        assert(x);  \
    }
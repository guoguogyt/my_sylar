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
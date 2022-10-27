/*
 * @Descripttion: 
 * @version: 
 * @Author: leileilei
 * @Date: 2022-10-26 16:12:58
 * @LastEditors: sueRimn
 * @LastEditTime: 2022-10-27 10:55:45
 */

#pragma once
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

namespace leileilei
{
    /**
     * @brief 
     *  是否开启hook
     * @return true 
     * @return false 
     */
    bool is_hook_enable();
    /**
     * @brief Set the hook enable object
     *  
     * @return true 
     * @return false 
     */
    void set_hook_enable(bool flag);
}

/**
 * @brief 
 *  使用C的编译风格
    因为C和C++的编译规则不一样，主要区别体现在编译期间生成函数符号的规则不一致。
    C++比C出道晚，但是增加了很多优秀的功能，函数重载就是其中之一。
    由于C++需要支持重载，单纯的函数名无法区分出具体的函数，所以在编译阶段就需要将形参列表作为附加项增加到函数符号中。
 */
extern "C"
{
// sleep  参数是秒
typedef unsigned int (*sleep_fun)(unsigned int seconds);
extern sleep_fun sleep_f;
// usleep 参数是微秒
typedef int (*usleep_fun)(useconds_t usec);
extern usleep_fun usleep_f;
// nanosleep 纳秒级别
typedef int (*nanosleep_fun)(const struct timespec *req, struct timespec *rem);
extern nanosleep_fun nanosleep_f;

}

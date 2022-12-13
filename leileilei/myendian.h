/*
 * @Descripttion: 
 * @version: 
 * @Author: leileilei
 * @Date: 2022-12-13 13:43:24
 * @LastEditors: sueRimn
 * @LastEditTime: 2022-12-13 14:33:15
 */
#pragma once

/**
 * 这个文件一开始叫做endian.h这个名称，在编译的时候一直出现已下问题：
 /usr/include/bits/waitstatus.h:79:27: error: redeclaration of ‘unsigned int wait::<anonymous struct>::__w_retcode’
  unsigned int __w_retcode:8;
                           ^
/usr/include/bits/waitstatus.h:74:27: note: previous declaration ‘unsigned int wait::<anonymous struct>::__w_retcode’
  unsigned int __w_retcode:8; // Return code if exited normally.  
                           ^
/usr/include/bits/waitstatus.h:80:28: error: redeclaration of ‘unsigned int wait::<anonymous struct>::__w_coredump’
  unsigned int __w_coredump:1;
                            ^
/usr/include/bits/waitstatus.h:73:28: note: previous declaration ‘unsigned int wait::<anonymous struct>::__w_coredump’
  unsigned int __w_coredump:1; // Set if dumped core.  
                            ^
/usr/include/bits/waitstatus.h:81:27: error: redeclaration of ‘unsigned int wait::<anonymous struct>::__w_termsig’
  unsigned int __w_termsig:7;
                           ^
/usr/include/bits/waitstatus.h:72:27: note: previous declaration ‘unsigned int wait::<anonymous struct>::__w_termsig’
  unsigned int __w_termsig:7; // Terminating signal.  
                           ^
/usr/include/bits/waitstatus.h:93:27: error: redeclaration of ‘unsigned int wait::<anonymous struct>::__w_stopsig’
  unsigned int __w_stopsig:8; // Stopping signal.  
                           ^
/usr/include/bits/waitstatus.h:88:27: note: previous declaration ‘unsigned int wait::<anonymous struct>::__w_stopsig’
  unsigned int __w_stopsig:8; // Stopping signal. 
                           ^
/usr/include/bits/waitstatus.h:94:27: error: redeclaration of ‘unsigned int wait::<anonymous struct>::__w_stopval’
  unsigned int __w_stopval:8; // W_STOPPED if stopped.  
                           ^
/usr/include/bits/waitstatus.h:87:27: note: previous declaration ‘unsigned int wait::<anonymous struct>::__w_stopval’
  unsigned int __w_stopval:8; // W_STOPPED if stopped.  

这个问题后面发现，在linux操作系统中有一个文件中有个名为endian.h的文件
这就会导致，当有系统中使用的某些头文件包含操作系统的endian.h文件时，会搞混，导致编译的并不是自己的endian.h文件

目前的解决方式:
    将自己的endian.h文件换一个名称，编译顺利通过
 */

#define LEI_LITTLE_ENDIAN 1
#define LEI_BIG_ENDIAN 2

#include <byteswap.h>
#include <stdint.h>

namespace leileilei
{

/**
 * @brief 8字节类型的字节序转化
 * @tparam T 
 * @param value 
 * @return std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type 
 */
template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type
byteswap(T value)
{
    return (T)bswap_64((uint64_t)value);
}

/**
 * @brief 4字节类型的字节序转化
 * @tparam T 
 * @param value 
 * @return std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type 
 */
template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type
byteswap(T value)
{
    return (T)bswap_32((uint32_t)value);
}

/**
 * @brief 2字节类型的字节序转化 
 * @tparam T 
 * @param value 
 * @return std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type 
 */
template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type
byteswap(T value)
{
    return (T)bswap_16((uint16_t)value);
}

#if BYTE_ORDER == BIG_ENDIAN
#define LEI_BYTE_ORDER LEI_BIG_ENDIAN
#else
#define LEI_BYTE_ORDER LEI_LITTLE_ENDIAN
#endif

// 当前机器是大端
#if LEI_BYTE_ORDER == LEI_BIG_ENDIAN
 template<class T>
T byteswapOnLittleEndian(T t)
{
    return t;
}

template<class T>
T byteswapOnBigEndian(T t)
{
    return byteswap(t);
} 

// 当前机器是小端
#else 
template<class T>
T byteswapOnLittleEndian(T t)
{
    return byteswap(t);
}

template<class T>
T byteswapOnBigEndian(T t)
{
    return t;
}
#endif

}

#pragma once


#define LEI_LITTLE_ENDIAN 1
#define LEI_BIG_ENDIAN 2

// #include <byteswap.h>
// #include <stdint.h>

namespace leileilei
{

// /**
//  * @brief 8字节类型的字节序转化
//  * @tparam T 
//  * @param value 
//  * @return std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type 
//  */
// template<class T>
// typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type
// byteswap(T value)
// {
//     return (T)bswap_64((uint64_t)value);
// }

// /**
//  * @brief 4字节类型的字节序转化
//  * @tparam T 
//  * @param value 
//  * @return std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type 
//  */
// template<class T>
// typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type
// byteswap(T value)
// {
//     return (T)bswap_32((uint32_t)value);
// }

// /**
//  * @brief 2字节类型的字节序转化 
//  * @tparam T 
//  * @param value 
//  * @return std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type 
//  */
// template<class T>
// typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type
// byteswap(T value)
// {
//     return (T)bswap_16((uint16_t)value);
// }

#if BYTE_ORDER == BIG_ENDIAN
#define LEI_BYTE_ORDER LEI_BIG_ENDIAN
#else
#define LEI_BYTE_ORDER LEI_LITTLE_ENDIAN
#endif

// // 当前机器是大端
// #if LEI_BYTE_ORDER == LEI_BIG_ENDIAN
//  template<class T>
// T byteswapOnLittleEndian(T t)
// {
//     return t;
// }

// template<class T>
// T byteswapOnBigEndian(T t)
// {
//     return byteswap(t);
// } 

// // 当前机器是小端
// #else 
// template<class T>
// T byteswapOnLittleEndian(T t)
// {
//     return byteswap(t);
// }

// template<class T>
// T byteswapOnBigEndian(T t)
// {
//     return t;
// }
// #endif

}

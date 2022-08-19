/**
 * @file nonecopy.h
 * @author your name (you@domain.com)
 * @brief 不可拷贝对象封装
 * @version 0.1
 * @date 2022-08-19
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#pragma once

namespace leileilei
{

/**
 * @brief 
 * 什么时候空的类不再是空的呢，当C++编译器处理过它之后。
    是这样的，如果你自己没声明什么，编译器就会为它声明一个拷贝构造、一个无参构造（默认构造）、赋值构造和析构函数（也就是big three）。
    如果你没有声明任何构造函数，编译器也会为你声明一个默认构造。所有这样函数都是public且inline（内联）。

    这些函数做了什么呢？默认构造和构造函数主要是给编译器一个地方用来放置藏身幕后的代码，像是唤起基类或叫父类非静态成员的构造函数与析构函数，
    编译器生产出来的析构函数是非虚函数，除非这个类的基类本身有虚析构函数。
    至于拷贝构造和赋值构造，编译器合成版只是单纯将源对象的每一个非静态数据成员拷贝到目的对象。

    如果你自行定义了一个构造函数，那么编译器就不会再给你一个默认构造函数。
    如果你强制加上 = default，就可以重新获得并使用默认构造函数。
    =delete告诉编译器不要定义它。必须出现在声明式。适用于任何成员函数。也适用于构造函数，但后果自负。

    这个类的作用是使得类禁止拷贝，赋值
 */
class NoneCopy
{
public:
    /**
     * @brief Construct a new None Copy object
     *  默认构造函数
     */
    NoneCopy() = default;
    /**
     * @brief Destroy the None Copy object
     * 默认的析构函数
     */
    ~NoneCopy() = default;
    /**
     * @brief Construct a new None Copy object
     * 拷贝构造函数(禁用)
     */
    NoneCopy(const NoneCopy&) = delete;
    /**
     * @brief 
     * 赋值函数(禁用)
     * @return NoneCopy& 
     */
    NoneCopy& operator=(const NoneCopy&) = delete;
};

}
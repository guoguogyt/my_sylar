#pragma once


#include <memory>

namespace leileilei
{

/*
    返回单利实例
        泛型X，N    暂时没用
*/
template<class T, class X = void, int N = 0>
class Singleton
{
    static T* GetInstance()
    {
        static T t;
        return &t;
    }
};

/*
    返回单利智能指针
*/
template<class T, class X = void, int N = 0>
class SingletonPtr
{
    static std::shared_ptr<T> GetInstancePtr()
    {
        std::shared_ptr<T> t(new T);
        return t;
    }
};



}
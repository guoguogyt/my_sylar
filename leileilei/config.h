#pragma once

#include <string>
#include <boost/lexical_cast.hpp>
#include <cxxabi.h>
#include <typeinfo>
#include "log.h"

namespace leileilei{

static leileilei::Logger::ptr logger_system = LEI_GET_LOGGER("system");

/**
 * @brief 获取泛型的名称
 * 
 * @tparam T 
 * @return const char* 
 */
template<class T>
const char* TypeToName()
{
    static const char* type_name = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
    return type_name;
}


/*
    配置项的基础类   
    包含配置项名称、配置项说明      没有加入配置项值，将这个类作为所有配置项的基类,所有配置项继承这个类，以后可以只关注配置项值层面

    包含三个需要被重写的虚函数
    toString()                             
    表示将配置项的值转换为string            
    fromString()
    表示将从string转换为配置项的值
    getConfName()
    获取配置项值得类型
*/
class ConfigVarBase
{
public:
    typedef std::shared_ptr<ConfigVarBase> ptr;
    ConfigVarBase(const std::string& name, const std::string& desc = "")
        :var_name_(name),var_desc_(desc)
    {
        /*
            将配置项名称转为小写
            这里转小写之后要注意，次系统的配置名是不区分大小写的，注意使用，否则会导致配置覆盖
        */
        std::transform(var_name_.begin(), var_name_.end(), var_name_.begin(), ::tolower);
    }
    virtual ~ConfigVarBase();//虚的析构函数

    std::string getName()   {   return var_name_;}
    std::string getDesc()   {   return var_desc_;}

    virtual std::string toString() = 0;
    virtual bool fromString() = 0;
    virtual std::string getConfName() = 0;
protected:
    std::string var_name_;//配置项名称
    std::string var_desc_;//配置项说明
};

/**
 * @brief 这个是一个最基础的类型T转换为V的类
            转化利用了boost库的中方法
        对运算符进行了重载，重载了操作符()
 *  之后会对这个类版本进行偏特化，使其可以支持常用的stl库类型
 * @tparam T 
 * @tparam V 
 */
template<class T, class V>
class LexicalCast
{
public:
    V operator()(const T& t)
    {
        /**
         * @brief 这个boost中的函数功能主要将   不同类型转换为对应的string类型，或者将string转化为T类型
         * 
         * @return return 
         */
        return boost::lexical_cast<V>(t);
    }
};


/*
    具体执行配置项值转对应类型，或者从string还原一个T类的配置项值
    这里偏特化两个LexicalCast类
*/
template<class T, class toStr = LexicalCast<T, std::string>, class fromStr = LexicalCast<std::string, T> >
class ConfigVar : public ConfigVarBase
{
public:
    typedef std::shared_ptr<ConfigVar> ptr;
    ConfigVar(const std::string& name, const T& value, const std::string& desc)
        : ConfigVarBase(name, desc), var_value_(value)
    {}

    /**
     * @brief 转为string
     *      利用boost库转为string
     * @return std::string 
     */
    std::string toString() override
    { 
        try
        {
            return toStr()(var_value_);
        }
        catch(std::exception& e)
        {
            LEI_LOG_ERROR(logger_system) << "ConfigVar::toString exception" << e.what() << "convert:" << TypeToName<T>() << "to string" << " name=" << var_name_;
        }
        return "";
    }

    bool fromString() override
    {

    }

    std::string getConfName() override
    {

    }

    const T getValue()  {   return var_value_;}
private:
    T var_value_;
};


}
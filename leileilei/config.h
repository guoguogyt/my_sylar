#pragma once

#include <string>
#include "log.h"

namespace leileilei{

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
private:
    std::string var_name_;//配置项名称
    std::string var_desc_;//配置项说明
};



/*

*/
template<class T>
class ConfigVar : public ConfigVarBase
{
public:
    ConfigVar(const std::strng& name, const T& value, const std::string& desc)
        : ConfigVarBase(name, desc), var_value_(value)
    {}

    std::string toString() override
    {

    }

    bool fromString() override
    {

    }

    std::string getConfName override
    {

    }
private:
    T var_value_;
};


}
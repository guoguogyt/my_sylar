#pragma once

#include <string>
#include <boost/lexical_cast.hpp>
#include <yaml-cpp/yaml.h>
#include <cxxabi.h>
#include <map>
#include <set>
#include <vector>
#include <list>
#include <unordered_set>
#include <unordered_map>
#include <typeinfo>
#include <functional>
#include "log.h"

namespace leileilei{

/**
 * @brief static：

生命周期：static修饰的局部变量只执行初始化一次，而且延长变量的生命周期，直到程序运行结束之后才释放。

可见性：修饰全局变量或函数时，该全局变量只能在本文件中访问，不能在其他文件中访问，即使是extern外部声明也不可以。函数也只能在本文件中调用，不能被其他文件调用。

存储方式：static修饰的变量存放在全局数据区的静态变量区，包括全局静态变量和局部静态变量

未经初始化的静态全局变量会被程序自动初始化为0（在函数体内声明的自动变量的值是随机的，除非它被显式初始化，而在函数体外被声明的自动变量也会被初始化为0）；
 * 
 */
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
    /**
     * @brief   #include <cxxabi.h>
                char* __cxa_demangle(const char *mangled_name, char *output_buffer, size_t *length, int *status)
        功能描述：
            __cxa_demangle是C++的函数，而且是ABI（应用程序二进制接口），
            用于将已被编译器转换后的函数名给还原为原来的形式（编译的时候加上-rdunamic选项，加入函数符号表才能正确显示），即进行符号重组。
            第一个参数mangled_name就是要进行符号重组的名称，后三个参数用处不大，可以不作理会
        返回值：
            __cxa_demangle的返回值是一个字符串指针，内容就是函数的原始名称
            （需要注意的是__cxa_demangle的返回值是调用malloc申请的内存空间，调用者必须手动释放它）

     */
    static const char* type_name = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
    return type_name;
}


/*
    配置项的基础类   
    包含配置项名称、配置项说明      没有加入配置项值，将这个类作为所有配置项的基类,所有配置项继承这个类，以后可以只关注配置项值层面
    名称、描述都是字符串型，而值的类型是多样化的，所以将名称和描述放一起，值利用泛型做到不同类型的兼容

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
            这里转小写之后要注意，本系统的配置名是不区分大小写的，注意使用，否则会导致配置覆盖
        */
        std::transform(var_name_.begin(), var_name_.end(), var_name_.begin(), ::tolower);
        //检查名称是否是符合规则的
        if(var_name_.find_first_not_of("qazwsxedcrfvtgbyhnujmikopl._0123456789") != std::string::npos)
        {
            LEI_LOG_ERROR(logger_system) << "ConfigVarBase::ConfigVarBase config name is error";
        }
    }
    virtual ~ConfigVarBase() {}; //虚的析构函数

    std::string getName()   {   return var_name_;}
    std::string getDesc()   {   return var_desc_;}

    virtual std::string toString() = 0;
    virtual bool fromString(std::string str) = 0;
    virtual std::string getConfType() = 0;
protected:
    std::string var_name_;//配置项名称
    std::string var_desc_;//配置项说明
};




/**
 * @brief 这个是一个最基础的类型T转换为V的类
            转化利用了boost库的中方法
        对运算符进行了重载，重载了操作符()
    之后会对这个类 进行偏特化，使其可以支持常用的stl库类型
    在 偏特化 的过程中使用了 YAML库 进行 中间转化
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
         *  boost::lexical_cast是实现配置系统的主力函数
         * @return return 
         */
        return boost::lexical_cast<V>(t);
    }
};

//偏特化类   std::string ---- std::vector<T>
template<class T>
class LexicalCast<std::string, std::vector<T> >
{
public:
    std::vector<T> operator()(const std::string& str)
    {
        YAML::Node node = YAML::Load(str);
        typename std::vector<T> vec;
        std::stringstream ss;
        for(int i=0;i<node.size();i++)
        {
            ss.str("");
            ss << node[i];
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};
//偏特化类   std::vector<T> ---- std::string
template<class T>
class LexicalCast<std::vector<T>, std::string>
{
public:
    std::string operator()(const std::vector<T>& var)
    {
        YAML::Node node(YAML::NodeType::Sequence);
        for(auto& it : var)
        {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(it)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};


//偏特化类   std::string ---- std::list<T>
template<class T>
class LexicalCast<std::string, std::list<T> >
{
public:
    std::list<T> operator()(const std::string& str)
    {
        YAML::Node node = YAML::Load(str);
        typename std::list<T> vec;
        std::stringstream ss;
        for(int i=0;i<node.size();i++)
        {
            ss.str("");
            ss << node[i];
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};
//偏特化类   std::list<T> ---- std::string
template<class T>
class LexicalCast<std::list<T>, std::string>
{
public:
    std::string operator()(const std::list<T>& var)
    {
        YAML::Node node(YAML::NodeType::Sequence);
        for(auto& it : var)
        {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(it)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

//偏特化类   std::string ---- std::set<T>
template<class T>
class LexicalCast<std::string, std::set<T> >
{
public:
    std::set<T> operator()(const std::string& str)
    {
        YAML::Node node = YAML::Load(str);
        typename std::set<T> vec;
        std::stringstream ss;
        for(int i=0;i<node.size();i++)
        {
            ss.str("");
            ss << node[i];
            vec.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};
//偏特化类   std::set<T> ---- std::string
template<class T>
class LexicalCast<std::set<T>, std::string>
{
public:
    std::string operator()(const std::set<T>& var)
    {
        YAML::Node node(YAML::NodeType::Sequence);
        for(auto& it : var)
        {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(it)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

//偏特化类   std::string ---- std::unordered_set<T>
template<class T>
class LexicalCast<std::string, std::unordered_set<T> >
{
public:
    std::unordered_set<T> operator()(const std::string& str)
    {
        YAML::Node node = YAML::Load(str);
        typename std::unordered_set<T> vec;
        std::stringstream ss;
        for(int i=0;i<node.size();i++)
        {
            ss.str("");
            ss << node[i];
            vec.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};
//偏特化类   std::unordered_set<T> ---- std::string
template<class T>
class LexicalCast<std::unordered_set<T>, std::string>
{
public:
    std::string operator()(const std::unordered_set<T>& var)
    {
        YAML::Node node(YAML::NodeType::Sequence);
        for(auto& it : var)
        {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(it)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

//偏特化类   std::string ---- std::map<std::string, T>
template<class T>
class LexicalCast<std::string, std::map<std::string, T> >
{
public:
    std::map<std::string, T> operator()(const std::string& str)
    {
        YAML::Node node = YAML::Load(str);
        typename std::map<std::string, T> vec;
        std::stringstream ss;
        for(auto it=node.begin(); it != node.end(); it++)
        {
            ss.str("");
            ss<< it->second;
            vec.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string, T>()(ss.str())));
        }
        return vec;
    }
};
//偏特化类   std::map<std::string,T> ---- std::string
template<class T>
class LexicalCast<std::map<std::string, T>, std::string>
{
public:
    std::string operator()(const std::map<std::string, T>& var)
    {
        YAML::Node node(YAML::NodeType::Map);
        for(auto& it : var)
        {
            node[it.first] = YAML::Load(LexicalCast<T, std::string>()(it.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

//偏特化类   std::string ---- std::unordered_map<std::string, T>
template<class T>
class LexicalCast<std::string, std::unordered_map<std::string, T> >
{
public:
    std::unordered_map<std::string, T> operator()(const std::string& str)
    {
        YAML::Node node = YAML::Load(str);
        typename std::unordered_map<std::string, T> vec;
        std::stringstream ss;
        for(auto it=node.begin(); it != node.end(); it++)
        {
            ss.str("");
            ss<< it->second;
            vec.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string, T>()(ss.str())));
        }
        return vec;
    }
};
//偏特化类   std::unordered_map<std::string,T> ---- std::string
template<class T>
class LexicalCast<std::unordered_map<std::string, T>, std::string>
{
public:
    std::string operator()(const std::unordered_map<std::string, T>& var)
    {
        YAML::Node node(YAML::NodeType::Map);
        for(auto& it : var)
        {
            node[it.first] = YAML::Load(LexicalCast<T, std::string>()(it.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
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
    typedef leileilei::RWMutex  RWMutexType;

    /**
     * @brief 
     *  可以对functional实体进行存储、复制、调用。这些实体包括Lambda表达式、函数指针、普通函数和其他函数对象等。
     其中函数指针是非类型安全的，functional可以对其实现类型安全的包裹。 
     */
    typedef std::function<void (const T& old_value, const T& new_value)> on_change_cb;

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
        RWMutexType::ReadLock readLock(mutex_);
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

    /**
     * @brief 将字符串转换为对应的类型
     * 
     * @param str 
     * @return true 
     * @return false 
     */
    bool fromString(std::string str) override
    {
        try
        {
            // var_value_ = fromStr()(str);
            setValue(fromStr()(str));
        }
        catch(std::exception& e)
        {
            LEI_LOG_ERROR(logger_system) << "ConfigVar::fromString exception " << e.what() << " convert: string to " << TypeToName<T>()  << " name = " << var_name_;
            return false;
        }
        return true;
    }

    std::string getConfType() override
    {
        return TypeToName<T>();
    }

    const T getValue()  
    {
        RWMutexType::ReadLock readLock(mutex_);   
        return var_value_;
    }

    bool setValue(const T& t)
    {
        //当值改变时调用所有的回调函数, 因为有!=判断，所以泛型T需要支持==符号
        {
            RWMutexType::ReadLock readLock(mutex_);
            if(var_value_ == t)
            {
                return true;
            }   
            else
            {
                for(auto& it : cb_funs)
                {
                    it.second(var_value_, t);
                }
            }
        }
        RWMutexType::WriteLock writeLock(mutex_);
        var_value_ = t;
        return true;
    }

    /**
     * @brief 当配置变更，及时作出处理
     *      利用回调函数的方式，针对不同的配置项作出不同的反应
     */
     // 增加回调关系
    bool addCallBack(on_change_cb func) 
    {
        static uint64_t s_fun_id = 0;
        RWMutexType::WriteLock writeLock(mutex_);
        s_fun_id++;
        cb_funs[s_fun_id] = func;
        return true;
    }
    
    //获取回调函数
    on_change_cb getCallBack(uint64_t key)
    {
        RWMutexType::ReadLock readLock(mutex_);
        return cb_funs.find(key) == cb_funs.end() ? nullptr : cb_funs[key];
    }

    //删除某个指定的回调
    on_change_cb delCallBack(uint64_t key)
    {
        RWMutexType::WriteLock writeLock(mutex_);
        cb_funs.erase(key);
    }

    //清除所有的回调函数
    void clearAllCallBack()
    {
        RWMutexType::WriteLock writeLock(mutex_);
        cb_funs.clear();
    }

private:
    T var_value_;
    std::map<uint64_t, on_change_cb> cb_funs;//存储回调函数
    RWMutexType mutex_;
};


/**
 * @brief 管理ConfigVar类
    提供方法可以创建/访问ConfigVar类
 * 
 */
class ConfigManager
{
public:
    typedef leileilei::RWMutex RWMutexType;
    typedef std::unordered_map<std::string, ConfigVarBase::ptr> ConfigVarMap;
    /**
     * @brief 根据配置项名称，找到其对应的配置项信息
     *          如果没有找到，则生成对应的配置项 

            只利用这个方法+偏特化LexicalCast类---可以实现部分配置功能(即不靠配置文件加载配置)
     * @tparam T 
     * @param name 
     * @param t 
     * @param desc 
     * @return ConfigVar<T>::ptr 
     */
    template<class T>
    static typename ConfigVar<T>::ptr LookUp(const std::string& name, const T& t, const std::string& desc = "")
    {
        {
            RWMutexType::ReadLock readLock(GetMutex());
            auto it = getConfigMap().find(name);
            if(it != getConfigMap().end())//如果可以找到这个配置
            {
                /**
                * @brief 这个函数可以将基类转换为派生类    只适用于智能指针
                * 
                */
                auto temp = std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
                if(temp)
                {
                    //转换成功
                    LEI_LOG_DEBUG(logger_system) << "find config, named " << name;
                    return temp;
                }
                else
                {
                    //转换失败
                    LEI_LOG_ERROR(logger_system) << "find config, named" << name << "  ,but this type[" << TypeToName<T>() << "] is not equal type[" << it->second->getConfType() << "]";
                    return nullptr;
                }
            }

            //创建新的配置项之前，判断配置项名称是否符合要求
            if(name.find_first_not_of("qazwsxedcrfvtgbyhnujmikolp._0123456789") != std::string::npos)
            {
                return nullptr;
            }
        }
        RWMutexType::WriteLock writeLock(GetMutex());
        //如果没有找到这个配置，则新生成一个配置
        typename ConfigVar<T>::ptr  cf(new ConfigVar<T>(name, t, desc));
        getConfigMap()[name] = cf;
        // LEI_LOG_DEBUG(logger_system) << "map size[" << getConfigMap().size() << "]";
        return cf;
    }

    /**
     * @brief 
     *  根据配置项名称找到对应的配置项
     * @param name 
     * @return ConfigVar<T>::ptr 
     */
    template<class T>
    static typename ConfigVar<T>::ptr LookUp(const std::string& name)
    {
        RWMutexType::ReadLock readLock(GetMutex());
        auto it = getConfigMap().find(name);
        if(it == getConfigMap().end())
        {
            return nullptr;
        }
        return std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
    }
    
    /**
     * @brief 载入YAML中的配置项
                    如果    没有    对应配置项则创建
                    如果    有      对应的配置项则覆盖
     * 
     * @param node 
     */
    static void LoadConfigFromYaml(const YAML::Node& node);

    /**
     * @brief 根据名称返回配置项
     * 
     * @param name 
     * @return ConfigVarBase 
     */
    static ConfigVarBase::ptr LookUpBase(const std::string& name);

    /**
     * @brief 
     * 遍历所有的配置项，并且可以传入对应的函数，操作每一个配置项
     * @param call_function 
     */
    static void Visit(std::function<void(ConfigVarBase::ptr)> call_function);
private:
    /**
     * @brief Get the Config Map object
     * 存放已有的配置项,这个属性因为在静态方法LookUp中被使用到，所以将这个属性封装成一个静态函数进行获取
     * 这里注意返回值应该是引用类型，否则相当于没有对name_config_进行修改
     * @return std::unordered_map<std::string, ConfigVarBase::ptr>& 
     */
    static ConfigVarMap& getConfigMap()
    {
        static ConfigVarMap  name_config_;
        return name_config_;
    }

    static RWMutexType& GetMutex()
    {
        static RWMutexType mutex_;
        return mutex_;
    } 
};

}
#include "config.h"

//不加需要在用类名时加，否则会报找不到定义的错误
namespace leileilei
{


/**
 * @brief 因为yaml的数据格式不是简单的字符串
 *          该函数先将，每个yaml配置的配置项名称解析出来，   前缀之间的层级利用.表示
 * @param prefix 
 * @param node 
 * @param all_node 
 */
static void ListAllYamlNode(const std::string& prefix, const YAML::Node& node, 
                            std::list<std::pair<std::string, const YAML::Node> >& all_node)
{
    //前缀名称规范检查
    if(prefix.find_first_not_of("qazwsxedcrfvtgbyhnujmikolp._1234567890") != std::string::npos)
    {
        LEI_LOG_ERROR(LEI_GET_LOGGER("system")) << "prefix[" << prefix << "] is not available"; 
        return ;
    }

    all_node.push_back(std::make_pair(prefix, node));

    if(node.IsMap())
    {
        for(auto it=node.begin(); it!=node.end(); it++)
        {
            //注意prefix变化
            ListAllYamlNode(prefix.empty() ? it->first.Scalar() : prefix+"."+it->first.Scalar(), it->second, all_node);
        }
    }
}



void ConfigManager::LoadConfigFromYaml(const YAML::Node& node)
{
    std::list<std::pair<std::string, const YAML::Node> > all_node;
    ListAllYamlNode("", node, all_node);
    // for(auto& it : all_node)
    // {
    //     LEI_LOG_DEBUG(LEI_GET_LOGGER("system")) << "prefix[" << it.first << "]-----node[" << it.second << "]";
    // }

    for(auto& it : all_node)
    {
        std::string key = it.first;
        if(key.empty()) continue;

        /**
         * @brief 
         *      约定大于配置，如果没有约定则直接舍弃
         */
        ConfigVarBase::ptr cf = LookUpBase(key);
        if(cf)
        {   
            LEI_LOG_DEBUG(LEI_GET_LOGGER("system")) << "key[" << key << "]";
            if(it.second.IsScalar())
            {
                cf->fromString(it.second.Scalar());
            }
            else
            {
                std::stringstream ss;
                ss << it.second;
                cf->fromString(ss.str());
            }
        }
    }
}




ConfigVarBase::ptr ConfigManager::LookUpBase(const std::string& name)
{
    auto it = getConfigMap().find(name);
    if(it == getConfigMap().end())
    {
        return nullptr;
    }
    return it->second;
}



}

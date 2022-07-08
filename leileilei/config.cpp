#include "config.h"


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
        LEI_LOG_ERROR(logger_system) << "prefix[" << prefix << "] is not available"; 
    }
    all_node.push_back(std::make_pair(prefix, node));
    LEI_LOG_DEBUG(logger_system) << "prefix = " << prefix << "--- node =" << node;

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
}




ConfigVarBase ConfigManager::LookUpBase(const std::string& name)
{

}

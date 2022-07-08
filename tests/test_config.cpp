#include <iostream>
#include <yaml-cpp/yaml.h>
#include "../leileilei/log.h"
#include "../leileilei/config.h"

//加载一个yaml
void print_yaml(const YAML::Node& node, int level)
{

}

int main(int argc, char* argv[])
{
    // leileilei::LogManager lm;
    // leileilei::Logger::ptr system = lm.getLogger("system");


    YAML::Node root = YAML::LoadFile("/root/share/my_sylar/bin/config/test.yml");
    //输出 0或1
    // LEI_LOG_DEBUG(system) << root["logs"].IsDefined();
    //输出配置名称为name 的值
    // LEI_LOG_DEBUG(system) << root["logs"];
    //输出整个root表示的配置或配置组
    // LEI_LOG_DEBUG(system) << root;
    // print_yaml(root, 0);

    // leileilei::ConfigVar<int>::ptr g_int_value_config(new leileilei::ConfigVar<int>("system.port", (int)8080, "this is system port"));
    // LEI_LOG_DEBUG(LEI_LOG_GETROOTOR()) << g_int_value_config->getName();
    // LEI_LOG_DEBUG(LEI_LOG_GETROOTOR()) << g_int_value_config->getValue();
    // LEI_LOG_DEBUG(LEI_LOG_GETROOTOR()) << g_int_value_config->getDesc();
    // LEI_LOG_DEBUG(LEI_LOG_GETROOTOR()) << g_int_value_config->toString();
    //修改int类型的值
    // g_int_value_config->fromString("8090");
    // LEI_LOG_DEBUG(LEI_LOG_GETROOTOR()) << g_int_value_config->getValue();
    // LEI_LOG_DEBUG(LEI_LOG_GETROOTOR()) << g_int_value_config->getConfType();

    //相当于在约定
    leileilei::ConfigVar<int>::ptr g_int_value_config = 
                            leileilei::ConfigManager::LookUp("system.port", (int)8080, "this is system port");
    leileilei::ConfigVar<float>::ptr g_float_value_config = 
                            leileilei::ConfigManager::LookUp("system.value", (float)3.1415, "this is system value");
    leileilei::ConfigVar<std::vector<int> >::ptr g_vector_value_config = 
                            leileilei::ConfigManager::LookUp("system.vector", std::vector<int>{1,2}, "this is system vector");
    leileilei::ConfigVar<std::list<int> >::ptr g_list_value_config = 
                            leileilei::ConfigManager::LookUp("system.list", std::list<int>{3,4}, "this is system list");
    leileilei::ConfigVar<std::set<int> >::ptr g_set_value_config = 
                            leileilei::ConfigManager::LookUp("system.set", std::set<int>{5,6}, "this is system set");
    leileilei::ConfigVar<std::unordered_set<int> >::ptr g_unset_value_config = 
                            leileilei::ConfigManager::LookUp("system.unset", std::unordered_set<int>{5,6}, "this is system unset");
    leileilei::ConfigVar<std::map<std::string, int> >::ptr g_map_value_config = 
                            leileilei::ConfigManager::LookUp("system.map", std::map<std::string, int>{{"a",1},{"b",2}}, "this is system map");
    leileilei::ConfigVar<std::unordered_map<std::string,int> >::ptr g_unmap_value_config = 
                            leileilei::ConfigManager::LookUp("system.unmap", std::unordered_map<std::string, int>{{"a",1},{"b",2}}, "this is system unmap");


    LEI_LOG_DEBUG(LEI_LOG_GETROOTOR()) << g_int_value_config->getDesc() << "    before:" << g_int_value_config->toString();
    LEI_LOG_DEBUG(LEI_LOG_GETROOTOR()) << g_float_value_config->getDesc() << "  before:" << g_float_value_config->toString();

#define XX(g_var, prefix) \
    { \
        auto& value = g_var->getValue(); \
        for(auto it : value) \
        { \
            LEI_LOG_DEBUG(LEI_LOG_GETROOTOR()) << g_var->getDesc() << "    " #prefix  "     value=" << it; \
        } \
    }

#define XXM(g_var, prefix) \
    { \
        auto& value = g_var->getValue(); \
        for(auto it : value) \
        { \
            LEI_LOG_DEBUG(LEI_LOG_GETROOTOR()) << g_var->getDesc() << "    " #prefix  "       key=" << it.first << "  value=" << it.second; \
        } \
    }

    XX(g_vector_value_config, before);
    XX(g_list_value_config, before);
    XX(g_set_value_config, before);
    XX(g_unset_value_config, before);
    XXM(g_map_value_config, before);
    XXM(g_unmap_value_config, before);


    leileilei::ConfigManager::LoadConfigFromYaml(root);

    LEI_LOG_DEBUG(LEI_LOG_GETROOTOR()) << g_int_value_config->getDesc() << "    after:" << g_int_value_config->toString();
    LEI_LOG_DEBUG(LEI_LOG_GETROOTOR()) << g_float_value_config->getDesc() << "  after:" << g_float_value_config->toString();







#undef XX
#undef XXM

    return 0;
}

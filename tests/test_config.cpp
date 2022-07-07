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


    leileilei::ConfigVar<int>::ptr g_int_value_config = leileilei::ConfigManager::LookUp("system.port", (int)8080, "this is system port");
    leileilei::ConfigVar<float>::ptr g_float_value_config = leileilei::ConfigManager::LookUp("system.value", (float)3.1415, "this is system value");



    LEI_LOG_DEBUG(LEI_LOG_GETROOTOR()) << g_int_value_config->getDesc() << "before:" << g_int_value_config->toString();
    LEI_LOG_DEBUG(LEI_LOG_GETROOTOR()) << g_float_value_config->getDesc() << "before:" << g_float_value_config->toString();

    return 0;
}

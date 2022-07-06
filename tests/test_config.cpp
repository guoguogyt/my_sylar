#include <iostream>
#include <yaml-cpp/yaml.h>
#include "../leileilei/log.h"

//加载一个yaml
void print_yaml(const YAML::Node& node, int level)
{

}

int main(int argc, char* argv[])
{
    leileilei::LogManager lm;
    leileilei::Logger::ptr system = lm.getLogger("system");


    YAML::Node root = YAML::LoadFile("/root/share/my_sylar/bin/config/test.yml");
    LEI_LOG_DEBUG(system) << root["logs"].IsDefined();
    LEI_LOG_DEBUG(system) << root["logs"];
    LEI_LOG_DEBUG(system) << root;
    // print_yaml(root, 0);
    return 0;
}

#include<iostream>
#include<yaml-cpp/yaml.h>

void print_yaml()
{

}

int main(int argc, char* argv[])
{
    YAML::Node root = YAML::LoadFile("/root/share/my_sylar/bin/config/test.yml");
    return 0;
}

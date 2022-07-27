#include <iostream>
#include <yaml-cpp/yaml.h>
#include "../leileilei/log.h"
#include "../leileilei/config.h"

//加载一个yaml
void print_yaml(const YAML::Node& node, int level)
{

}

int test_stl_config_yaml()
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


    XX(g_vector_value_config, after);
    XX(g_list_value_config, after);
    XX(g_set_value_config, after);
    XX(g_unset_value_config, after);
    XXM(g_map_value_config, after);
    XXM(g_unmap_value_config, after);

#undef XX
#undef XXM

    return 0;
}


class Person
{
public:
    Person(){};
    std::string name;
    int age;
    bool sex;

    /**
     * @brief  error: passing ‘const Person’ as ‘this’ argument of ‘std::string Person::to_string()’ discards qualifiers [-fpermissive]
        一个const对象不能调用非const成员函数，即使成员函数并没有改变对象成员的值，编译器也会以为其改变了对象。
        所以要想调用那个函数，就（只能？）把那个函数设成const函数，也就是在函数后加const，以显式的告诉编译器，这个函数是类内的静态函数，不能改变类的成员变量。
     */
    std::string to_string() const
    {
        std::stringstream ss;
        ss << "[Class-Person  name-"<<name
            <<" age-"<<age
            <<" sex"<<sex
            <<"]";
        return ss.str();
    }

    bool operator==(const Person& p) const
    {
        return name==p.name && age==p.age && sex==p.sex;
    }
};

namespace leileilei
{
//偏特化类   Person <----> std::string
template<>
class LexicalCast<std::string, Person>
{
public:
    Person operator()(const std::string& str)
    {
        YAML::Node node = YAML::Load(str);
        Person per;
        per.name = node["name"].as<std::string>();
        per.age = node["age"].as<int>();
        per.sex = node["sex"].as<bool>();

        return per;
    }
};
template<>
class LexicalCast<Person, std::string>
{
public:
    std::string operator()(Person var)
    {
        YAML::Node node;
        node["name"] = var.name;
        node["age"] = var.age;
        node["sex"] = var.sex;
        std::stringstream ss;
        ss<< node;

        return ss.str();
    }
};

}

void test_config_yaml_class()
{
    leileilei::ConfigVar<Person>::ptr g_person_value_config = 
                            leileilei::ConfigManager::LookUp("system.person", Person(), "this is system person");
    leileilei::ConfigVar<std::map<std::string, Person> >::ptr g_personmap_value_config = 
                            leileilei::ConfigManager::LookUp("system.personmap", std::map<std::string, Person>(), "this is system personmap");
    leileilei::ConfigVar<std::map<std::string, std::vector<Person> > >::ptr g_personmapvec_value_config = 
                            leileilei::ConfigManager::LookUp("system.personmapvec", std::map<std::string, std::vector<Person> >(), "this is system personmapvec");

    LEI_LOG_DEBUG(LEI_LOG_GETROOTOR()) << "person before - " << g_person_value_config->getValue().to_string() << " - " << g_person_value_config->toString();
#define XX_PERSON(g_var, prefix) \
    { \
        auto vlu = g_var->getValue(); \
        for(auto& i : vlu) \
        { \
            LEI_LOG_DEBUG(LEI_LOG_GETROOTOR()) << prefix << "   " << i.first << "-" << i.second.to_string(); \
        } \
    }

    g_person_value_config->addCallBack([](const Person& old_person, const Person& new_person)
        {LEI_LOG_DEBUG(LEI_LOG_GETROOTOR())<< "doing config callback!";}
    );

    XX_PERSON(g_personmap_value_config, "  personmap before  ");
    LEI_LOG_DEBUG(LEI_LOG_GETROOTOR()) << "personmapvec before - " <<  g_personmapvec_value_config->toString();


    YAML::Node root = YAML::LoadFile("/root/share/my_sylar/bin/config/test.yml");
    leileilei::ConfigManager::LoadConfigFromYaml(root);


    LEI_LOG_DEBUG(LEI_LOG_GETROOTOR()) << "person after - " << g_person_value_config->getValue().to_string() << " - " << g_person_value_config->toString();
    XX_PERSON(g_personmap_value_config, "  personmap after  ");
    LEI_LOG_DEBUG(LEI_LOG_GETROOTOR()) << "personmapvec after - " <<  g_personmapvec_value_config->toString();

#undef XX_PERSON
}


void test_log_yaml()
{
    static leileilei::Logger::ptr system_log = LEI_GET_LOGGER("testLogger2");
    LEI_LOG_DEBUG(system_log) << "hello system" << std::endl;
    // std::cout << sylar::LoggerMgr::GetInstance()->toYamlString() << std::endl;
    
    std::cout << "=============" << std::endl;
    YAML::Node root = YAML::LoadFile("/root/share/my_sylar/bin/config/log.yml");
    leileilei::ConfigManager::LoadConfigFromYaml(root);
    // std::cout << sylar::LoggerMgr::GetInstance()->toYamlString() << std::endl;
    std::cout<< "new logger-" << system_log->getLoggerName()
                                 << "  format-"<<  system_log->getAppender(0)->getFormat()->getFormat()<<std::endl;
    std::cout << "=============" << std::endl;
    // std::cout << root << std::endl;
    LEI_LOG_DEBUG(system_log) << "hello system" << std::endl;

    // system_log->setFormatter("%d - %m%n");
    // SYLAR_LOG_INFO(system_log) << "hello system" << std::endl;
}

int main(int argc,char* argv[])
{
    // test_stl_config_yaml();
    // test_config_yaml_class();
    test_log_yaml();
}

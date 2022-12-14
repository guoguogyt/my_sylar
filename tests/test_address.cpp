#include "leileilei.h"

leileilei::Logger::ptr g_logger = LEI_GET_LOGGER("system");

void test()
{
    std::vector<leileilei::Address::ptr> addrs;

    LEI_LOG_DEBUG(g_logger) << "begin";

    bool v = leileilei::Address::Lookup(addrs, "localhost:3080");

    LEI_LOG_DEBUG(g_logger) << "end";

    if(!v)
    {
        LEI_LOG_ERROR(g_logger) << "lookup fail";
        return;
    }

    for(size_t i=0; i<addrs.size(); i++)
    {
        LEI_LOG_DEBUG(g_logger) << i << " - " << addrs[i]->toString();
    }

    auto addr = leileilei::Address::LookupAny("localhost:4080");
    if(addr)
        LEI_LOG_DEBUG(g_logger) << *addr;
    else
        LEI_LOG_ERROR(g_logger) << "error";
}

void test_iface()
{
    std::multimap<std::string, std::pair<leileilei::Address::ptr, uint32_t> > results;

    bool v = leileilei::Address::GetInterfaceAddresses(results);
    if(!v)
    { 
        LEI_LOG_ERROR(g_logger) << "GetInterfaceAddresses fail";
        return;
    }

    for(auto& i:results)
    {
        LEI_LOG_DEBUG(g_logger) << i.first << " - " << i.second.first->toString() << " - " << i.second.second;
    }
}

void test_ipv4()
{
    auto addr = leileilei::IPv4Address::Create("127.0.0.1");
    if(addr)
        LEI_LOG_DEBUG(g_logger) << addr->toString();
    LEI_LOG_DEBUG(g_logger) << addr->addr_.sin_addr.s_addr;
}

int main(int argc, char* argv[])
{
    test_ipv4();
    return 0;
}
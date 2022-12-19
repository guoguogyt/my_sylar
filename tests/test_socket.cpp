/*
 * @Descripttion: 
 * @version: 
 * @Author: leileilei
 * @Date: 2022-12-19 09:22:38
 * @LastEditors: sueRimn
 * @LastEditTime: 2022-12-19 11:36:26
 */
#include "leileilei.h"

static leileilei::Logger::ptr g_logger = LEI_GET_LOGGER("system");

void test_socket()
{
    leileilei::IPAddress::ptr addr = leileilei::Address::LookupAnyIPAddress("www.baidu.com");
    if(addr)
        LEI_LOG_DEBUG(g_logger) << "get address: " << addr->toString();
    else
        LEI_LOG_ERROR(g_logger) << "get address fail";
    
    leileilei::Socket::ptr sock = leileilei::Socket::CreateTCP(addr);
    addr->setPort(80);
    LEI_LOG_DEBUG(g_logger) << "addr=" << addr->toString();

    if(sock->connect(addr))
    {
        LEI_LOG_ERROR(g_logger) << "connect " << addr->toString() << " fail";
        return; 
    }
    else
        LEI_LOG_DEBUG(g_logger) << "Connect " << addr->toString() << "connected";

    const char buff[] = "GET / HTTP/1.0\r\n\r\n";
    int rt = sock->send(buff, sizeof(buff));
    if(rt <= 0)
    {
        LEI_LOG_DEBUG(g_logger) << "send fail rt=" << rt;
        return;
    }

    std::string buffs;
    buffs.resize(4096);
    rt = sock->recv(&buffs[0], buffs.size());

    if(rt <= 0)
    {
        LEI_LOG_ERROR(g_logger) << "recv fail rt=" <<rt;
        return;
    }
    buffs.resize(rt);
    LEI_LOG_DEBUG(g_logger) << buffs;
}

void test2()
{
    leileilei::IPAddress::ptr addr = leileilei::Address::LookupAnyIPAddress("www.baidu.com:80");
    if(addr)
        LEI_LOG_DEBUG(g_logger) << "get address: " << addr->toString();
    else
    {
        LEI_LOG_ERROR(g_logger) << "get address fail";
        return;
    }
    
    leileilei::Socket::ptr sock = leileilei::Socket::CreateTCP(addr);
    if(!sock->connect(addr))
    {
        LEI_LOG_ERROR(g_logger) << "connect " << addr->toString() << " fail";
        return;
    }
    else
        LEI_LOG_DEBUG(g_logger) << "connect " << addr->toString() << "connected";
    
    uint64_t ts = leileilei::GetCurrentUS();
    for(size_t i=0; i<10000000000ul; i++)
    {
        if(int err = sock->getError())
        {
            LEI_LOG_DEBUG(g_logger) << "err=" << err << " errstr=" <<strerror(err);
            break;
        }

        static int batch = 10000000;
        if(i && (i%batch) == 0)
        {
            uint64_t ts2 = leileilei::GetCurrentUS();
            LEI_LOG_DEBUG(g_logger) << "i=" << i << " used: " << ((ts2 - ts) * 1.0 / batch) << " us";
            ts = ts2;
        }
    }
}

int main(int argc, char* argv[])
{
    leileilei::IOManager iom;
    iom.schedule(&test_socket);
    return 0;
}
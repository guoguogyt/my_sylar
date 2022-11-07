/*
 * @Descripttion: 
 * @version: 
 * @Author: leileilei
 * @Date: 2022-10-27 09:32:16
 * @LastEditors: sueRimn
 * @LastEditTime: 2022-11-07 16:09:12
 */
#include "leileilei.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

leileilei::Logger::ptr g_logger = LEI_LOG_GETROOTOR();

void test_sleep()
{
    leileilei::IOManager iom(1);
    /**
     * @brief 
     * 这么使用，不会先sleep2s，在sleep10s
        会按照时间最长的sleep，既总休息10s
        因为可以当做schedule函数同时进入fiber list
        fiber list之间的执行也是连续的
     */
    iom.schedule([](){
        sleep(2);
        LEI_LOG_DEBUG(g_logger) << "sleep 2";
    });
    iom.schedule([](){
        sleep(10);
        LEI_LOG_DEBUG(g_logger) << "sleep 10";
    });
    LEI_LOG_DEBUG(g_logger) << "test sleep";
}

void test_sock()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "112.80.248.75", &addr.sin_addr.s_addr);

    LEI_LOG_DEBUG(g_logger) << "begin connct";

    int rt = connect(sock, (const sockaddr*)&addr, sizeof(addr));
    LEI_LOG_DEBUG(g_logger) << "connect rt = " << rt << "   errno=" << errno;

    if(rt)
        return;
    
    const char data[] = "GET / HTTP/1.0\r\n\r\n";
    rt = send(sock, data, sizeof(data), 0);
    LEI_LOG_DEBUG(g_logger) << "send rt=" << rt << "    errno=" << errno;
    if(rt<=0)
        return;

    std::string buff;
    buff.resize(4096);

    rt =recv(sock, &buff[0], buff.size(), 0);
    LEI_LOG_DEBUG(g_logger) << "recv rt=" << rt << "    errno=" << errno;
    if(rt<=0)
        return;
    
    buff.resize(rt);
    LEI_LOG_DEBUG(g_logger) << buff;
}

int main()
{
    // test_sleep();
    leileilei::IOManager iom;
    iom.schedule(test_sock);
    return 0;
}


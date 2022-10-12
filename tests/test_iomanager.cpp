/*
 * @Descripttion: 
 * @version: 
 * @Author: leileilei
 * @Date: 2022-09-27 11:23:18
 * @LastEditors: sueRimn
 * @LastEditTime: 2022-10-12 11:38:45
 */
#include "leileilei.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>

static leileilei::Logger::ptr g_logger = LEI_GET_LOGGER("system");

int sock = 0;

void test_fun()
{
    LEI_LOG_DEBUG(g_logger) << "test_fun sock=" << sock;

    //sleep(3);

    //close(sock);
    //sylar::IOManager::GetThis()->cancelAll(sock);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "112.80.248.75", &addr.sin_addr.s_addr);

    if(!connect(sock, (const sockaddr*)&addr, sizeof(addr))) {
    } else if(errno == EINPROGRESS) {
        LEI_LOG_DEBUG(g_logger) << "add event errno=" << errno << " " << strerror(errno);
        leileilei::IOManager::GetThis()->addEvent(sock, leileilei::IOManager::READ, [](){
            LEI_LOG_DEBUG(g_logger) << "read callback";
        });
        leileilei::IOManager::GetThis()->addEvent(sock, leileilei::IOManager::WRITE, [](){
            LEI_LOG_DEBUG(g_logger) << "write callback";
            //close(sock);
            leileilei::IOManager::GetThis()->cancelEvent(sock, leileilei::IOManager::READ);
            close(sock);
        });
    } else {
        LEI_LOG_DEBUG(g_logger) << "else " << errno << " " << strerror(errno);
    }}

void test1()
{
    leileilei::IOManager iom(2, false, "");
    sleep(3000);
    iom.schedule(&test_fun);
}


leileilei::Timer::ptr s_timer;
void test_timer()
{
    leileilei::IOManager iom(2);
    s_timer = iom.addTimer(1000, [](){
        LEI_LOG_DEBUG(g_logger) << "do timer  " << leileilei::GetCurrentMS();
    }, true);
}

int main(int argc, char* argv[])
{
    // test1();
    test_timer();
    return 0;
}
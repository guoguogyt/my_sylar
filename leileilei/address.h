/*
 * @Descripttion: 
 * @version: 
 * @Author: leileilei
 * @Date: 2022-11-24 15:53:58
 * @LastEditors: sueRimn
 * @LastEditTime: 2022-12-13 14:56:41
 */
#pragma once

#include <memory>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <map>

/**
sockaddr其定义如下：
struct sockaddr {
    unsigned short sa_family; // address family, AF_xxx   2byte
    char sa_data[14]; // 14 bytes of protocol address       14byte
};
sa_family ：是2字节的地址家族，一般都是“AF_xxx”的形式，它的值包括三种：AF_INET，AF_INET6和AF_UNSPEC。
如果指定AF_INET，那么函数就不能返回任何IPV6相关的地址信息；如果仅指定了AF_INET6，则就不能返回任何IPV4地址信息。
AF_UNSPEC则意味着函数返回的是适用于指定主机名和服务名且适合任何协议族的地址。
如果某个主机既有AAAA记录(IPV6)地址，同时又有A记录(IPV4)地址，那么AAAA记录将作为sockaddr_in6结构返回，而A记录则作为sockaddr_in结构返回
通常用的都是AF_INET。

sockaddr_in其定义如下：
struct sockaddr_in {
    short int sin_family; // Address family 2bytye
    unsigned short int sin_port; // Port number 2byte
    struct in_addr sin_addr; // Internet address  4byte
    unsigned char sin_zero[8]; // Same size as struct sockaddr 8byte
};
sin_family：指代协议族，在socket编程中只能是AF_INET
sin_port：存储端口号（使用网络字节顺序）
sin_addr：存储IP地址，使用in_addr这个数据结构
sin_zero：是为了让sockaddr与sockaddr_in两个数据结构保持大小相同而保留的空字节。
而其中in_addr结构的定义如下：
typedef struct in_addr {
    union {
        struct{ unsigned char s_b1,s_b2, s_b3,s_b4;} S_un_b;
        struct{ unsigned short s_w1, s_w2;} S_un_w;
        unsigned long S_addr;
    } S_un;
} IN_ADDR;
阐述下in_addr的含义，很显然它是一个存储ip地址的共用体有三种表达方式：
第一种用四个字节来表示IP地址的四个数字；
第二种用两个双字节来表示IP地址；
第三种用一个长整型来表示IP地址。
给in_addr赋值的一种最简单方法是使用inet_addr函数，它可以把一个代表IP地址的字符串赋值转换为in_addr类型，如addrto.sin_addr.s_addr=inet_addr("192.168.0.2");
其反函数是inet_ntoa，可以把一个in_addr类型转换为一个字符串。



addrinfo结构主要在网络编程解析hostname时使用，其在头文件#include<netdb.h>中，定义如下：
struct addrinfo {
    int ai_flags;   // AI_PASSIVE, AI_CANONNAME, AI_NUMERICHOST 
    int ai_family;  // PF_xxx 
    int ai_socktype;    // SOCK_xxx 
    int ai_protocol;    // 0 or IPPROTO_xxx for IPv4 and IPv6 
    socklen_t ai_addrlen;   // length of ai_addr
    char    *ai_canonname;  // canonical name for hostname 
    struct  sockaddr *ai_addr;  // binary address 
    struct  addrinfo *ai_next;  // next structure in linked list 
} ADDRINFOA, *PADDRINFOA;
ai_flags：
AI_PASSIVE 套接字地址将用于调用bind 函数
AI_CANONNAME 返回规范名称
AI_NUMERICHOST 传递给getaddrinfo函数的nodename参数必须是数字字符串。
AI_ALL If this bit is set, a request is made for IPv6 addresses and IPv4 addresses with AI_V4MAPPED.
AI_ADDRCONFIG 只有配置了全局地址后，getaddrinfo才会解析。 IPv6和IPv4环回地址不被认为是有效的全局地址。
AI_V4MAPPED 如果对IPv6地址的getaddrinfo请求失败，则对IPv4地址进行名称服务请求，这些地址将转换为IPv4映射IPv6地址格式。
AI_NON_AUTHORITATIVE 地址信息可以来自非授权命名空间提供商
AI_SECURE 地址信息来自安全信道。
AI_RETURN_PREFERRED_NAMES 地址信息是用于用户的优选名称。
AI_FQDN getaddrinfo将返回名称最终解析为的完全限定域名。 完全限定域名在ai_canonname成员中返回。
这与AI_CANONNAME位标记不同，后者返回在DNS中注册的规范名称，该名称可能与平面名称解析为的完全限定域名不同。
只能设置AI_FQDN和AI_CANONNAME位中的一个。 如果EAI_BADFLAGS同时存在这两个标志，getaddrinfo函数将失败。
AI_FILESERVER 命名空间提供程序提示正在查询的主机名正在文件共享方案中使用。 命名空间提供程序可以忽略此提示。

ai_family: 
AF_UNSPEC 地址系列未指定。
AF_INET IPv4 address family.
AF_NETBIOS NetBIOS地址系列。
AF_INET6 IPv6 address family.
AF_IRDA The Infrared Data Association address family.
AF_BTH Bluetooth address family.

ai_protocol: 协议类型。
IPPROTO_TCP 传输控制协议（TCP）。 当ai_family成员为AF_INET或AF_INET6且ai_socktype成员为SOCK_STREAM时，这是一个可能的值
IPPROTO_UDP 用户数据报协议（UDP）。 当ai_family成员为AF_INET或AF_INET6且类型参数为SOCK_DGRAM时，这是一个可能的值。
IPPROTO_RM PGM协议用于可靠的组播。 当ai_family成员为AF_INET且ai_socktype成员为SOCK_RDM时，这是一个可能的值。 
在为Windows Vista及更高版本发布的Windows SDK上，此值也称为IPPROTO_PGM。
可能的选项特定于指定的地址系列和套接字类型。
如果为ai_protocol指定了值0，则调用者不希望指定协议，服务提供者将选择要使用的ai_protocol。 对于IPv4和IPv6之外的协议，将ai_protocol设置为零。
下表列出了ai_protocol成员的通用值，尽管其他许多值也是可能的。

ai_socktype:　　套接字类型
SOCK_STREAM 使用OOB数据传输机制提供顺序，可靠，双向，基于连接的字节流。
使用Internet地址系列（AF_INET或AF_INET6）的传输控制协议（TCP）。如果ai_family成员是AF_IRDA，则SOCK_STREAM是唯一支持的套接字类型。
SOCK_DGRAM 支持数据报，它是无连接的，不可靠的固定（通常小）最大长度的缓冲区。对Internet地址系列（AF_INET或AF_INET6）使用用户数据报协议（UDP）。
SOCK_RAW 提供一个原始套接字，允许应用程序处理下一个上层协议头。要操作IPv4标头，必须在套接字上设置IP_HDRINCL套接字选项。
要操作IPv6头，必须在套接字上设置IPV6_HDRINCL套接字选项。
SOCK_RDM 提供可靠的消息数据报。这种类型的示例是在Windows中的实用通用多播（PGM）多播协议实现，通常被称为可靠多播节目。
SOCK_SEQPACKET 基于数据报提供伪流包。


#include <sys/socket.h>
#include <netdb.h>
int getaddrinfo(const char *restrict nodename, // host 或者IP地址 
    const char *restrict servname, // 十进制端口号 或者常用服务名称如"ftp"、"http"等 
    const struct addrinfo *restrict hints, // 获取信息要求设置 
    struct addrinfo **restrict res); // 获取信息结果 
nodename
主机名("www.baidu.com")或者是数字化的地址字符串(IPv4的点分十进制串("192.168.1.100")或者IPv6的16进制串("2000::1:2345:6789:abcd"))，
如果 ai_flags 中设置了AI_NUMERICHOST 标志，那么该参数只能是数字化的地址字符串，不能是域名，
该标志的作用就是阻止进行域名解析。
nodename 和 servname 可以设置为NULL，但是同时只能有一个为NUL。
servname
如果 ai_flags 设置了AI_NUMERICSERV 标志并且该参数未设置为NULL，那么该参数必须是一个指向10进制的端口号字符串，
不能设定成服务名，该标志就是用来阻止服务名解析。
hints
该参数指向用户设定的 struct addrinfo 结构体，只能设定该结构体中 ai_family、ai_socktype、ai_protocol 和 ai_flags 四个域，
其他域必须设置为0 或者 NULL, 通常是申请 结构体变量后使用memset()初始化再设定指定的四个域。
res
该参数获取一个指向存储结果的 struct addrinfo 结构体列表，使用完成后调用 freeaddrinfo() 释放存储结果空间。
void freeaddrinfo(struct addrinfo *ai);

如果 getaddrinfo() 函数执行成功，返回值为 0 ， 其他情况返回值表示错误种别。
使用函数gai_strerror() 可以获取可读性的错误信息,用法用strerror()相同，


struct ifaddrs
{
    struct ifaddrs  *ifa_next;    // Next item in list 
    char            *ifa_name;    // Name of interface 
    unsigned int     ifa_flags;   // Flags from SIOCGIFFLAGS 
    struct sockaddr *ifa_addr;    // Address of interface 
    struct sockaddr *ifa_netmask; // Netmask of interface 
    union
    {
        struct sockaddr *ifu_broadaddr; // Broadcast address of interface 
        struct sockaddr *ifu_dstaddr; // Point-to-point destination address 
    } ifa_ifu;
    #define              ifa_broadaddr ifa_ifu.ifu_broadaddr
    #define              ifa_dstaddr   ifa_ifu.ifu_dstaddr
    void            *ifa_data;    // Address-specific data 
}; 

getifaddrs()函数
头文件：
#include <sys/types.h>
#include <ifaddrs.h>
函数说明：获取本地网络接口的信息。在路由器上可以用这个接口来获取wan/lan等接口当前的ip地址，广播地址等信息。
int getifaddrs(struct ifaddrs **ifap);
getifaddrs创建一个链表，链表上的每个节点都是一个struct ifaddrs结构，getifaddrs()返回链表第一个元素的指针

返回值：成功返回0, 失败返回-1,同时errno会被赋允相应错误码。


 */

namespace leileilei
{
class IPAddress;

class Address
{
public:
    typedef std::shared_ptr<Address> ptr;
    /**
     * @brief 通过sockaddr指针创建Address
     * @param addr sockaddr指针
     * @param addrlen sockaddr的长度
     * @return Address::ptr 返回和sockaddr相匹配的Address，失败返回nullptr
     */
    static Address::ptr Create(const sockaddr* addr, socklen_t addrlen);
    /**
     * @brief 通过host地址返回对应条件的所有Address 
     * @param result 保存满足条件的所有Address
     * @param host 域名，服务器名称，例：www.baidu.com
     * @param family 协议族(AF_INET,AF_INET6, AF_UNIX)
     * @param type socket类型   SOCK_STREAM、SOCK_DGRAM等
     * @param protocol 协议,IPPROTO_TCP、IPPROTO_UDP
     * @return true 
     * @return false 
     */
    static bool Lookup(std::vector<Address::ptr>& result, const std::string& host, int family = AF_INET, int type = 0, int protocol = 0);
    
    /**
     * @brief 通过host地址返回任意一个满足条件的Address 
     * @param host 域名，服务器名称，例：www.baidu.com
     * @param family 协议族(AF_INET,AF_INET6, AF_UNIX)
     * @param type socket类型   SOCK_STREAM、SOCK_DGRAM等
     * @param protocol 协议,IPPROTO_TCP、IPPROTO_UDP
     * @return Address::ptr 
     */
    static Address::ptr LookupAny(const std::string& host, int family = AF_INET, int type = 0, int protocol = 0);
    /**
     * @brief 通过host地址返回任意一个满足条件的IPAddress
     * @param host 域名，服务器名称，例：www.baidu.com
     * @param family 协议族(AF_INET,AF_INET6, AF_UNIX)
     * @param type socket类型   SOCK_STREAM、SOCK_DGRAM等
     * @param protocol 协议,IPPROTO_TCP、IPPROTO_UDP
     * @return Address::ptr 
     */
    static std::shared_ptr<IPAddress> LookupAnyIPAddress(const std::string& host, int family = AF_INET, int type = 0, int protocol = 0); 

    /**
     * @brief 返回本机所有网卡的<网卡名，地址，子网掩码位数>
     * @param result 保存本机所有地址
     * @param family 协议族(AF_INET,AF_INET6, AF_UNIX)
     * @return true 
     * @return false 
     */
    static bool GetInterfaceAddresses(std::multimap<std::string, std::pair<Address::ptr, uint32_t> >& result, int family = AF_INET);
    /**
     * @brief 返回指定网卡的<网卡名，地址，子网掩码位数>
     * @param result 保存指定网卡所有地址
     * @param iface 网卡名称
     * @param family 协议族(AF_INET,AF_INET6, AF_UNIX)
     * @return true 
     * @return false 
     */
    static bool GetInterfaceAddresses(std::vector<std::pair<Address::ptr, uint32_t> >& result, const std::string& iface, int family = AF_INET);

    /**
     * 虚的析构函数(会被继承) 
     */
    virtual ~Address();

    /**
     * @brief 返回协议族
     * @return int 
     */
    int getFamily() const;
    /**
     * @brief 返回sockaddr指针，只读
     * @return const sockaddr* 
     */
    virtual const sockaddr* getAddr() const = 0;
    /**
     * @brief 返回sockaddr指针，读写
     * @return const sockaddr* 
     */
    virtual sockaddr* getAddr() = 0;
    /**
     * @brief 返回sockaddr地址的长度
     * @return socklen_t 
     */
    virtual socklen_t getAddrLen() const = 0;
    /**
     * @brief 可读性输出地址 
     * @param os 
     * @return std::ostream& 
     */
    virtual std::ostream& insert(std::ostream& os) const = 0;
    /**
     * @brief 返回可读性字符串 
     * @return std::string 
     */
    std::string toString() const;
    /**
     * @brief 小于号比较函数 
     * @param rhs 
     * @return true 
     * @return false 
     */
    bool operator<(const Address& rhs) const;
    /**
     * @brief 等于号比较函数 
     * @param rhs 
     * @return true 
     * @return false 
     */
    bool operator==(const Address& rhs) const;
    /**
     * @brief 不等于号比较函数 
     * @param rhs 
     * @return true 
     * @return false 
     */
    bool operator!=(const Address& rhs) const;
};

// 接口类
class IPAddress : public Address
{
public:
    typedef std::shared_ptr<IPAddress> ptr;
    /**
     * @brief 通过域名,IP,服务器创建IPAdress 
     * @param address 域名,IP,服务器名等.举例: www.baidu.com
     * @param port 端口号
     * @return IPAddress::ptr 
     */
    static IPAddress::ptr Create(const char* address, uint16_t port = 0);
    /**
     * @brief 获取广播地址
     * @param prefix_len 
     * @return IPAddress::ptr 
     */
    virtual IPAddress::ptr broadcastAddress(uint32_t prefix_len) = 0;
    /**
     * @brief 获取网络地址
     * @param prefix_len 
     * @return IPAddress::ptr 
     */
    virtual IPAddress::ptr networdAddress(uint32_t prefix_len) = 0;
    /**
     * @brief 获取子网掩码地址 
     * @param prefix_len 
     * @return IPAddress::ptr 
     */
    virtual IPAddress::ptr subnetMask(uint32_t prefix_len) = 0;
    /**
     * @brief 获取端口号
     * @return uint32_t 
     */
    virtual uint32_t getPort() const = 0;
    /**
     * @brief 设置端口号
     * @param v 
     */
    virtual void setPort(uint16_t v) = 0;
};

class IPv4Address : public IPAddress
{
public:
    typedef std::shared_ptr<IPv4Address> ptr;
    /**
     * @brief 使用点分十进制创建IPv4Address
     * @param address 点分十进制地址, 如： 192.31.195.101
     * @param port 端口号
     * @return IPv4Address::ptr 
     */
    static IPv4Address::ptr Create(const char* address, uint16_t port = 0);
    /**
     * @brief 通过sockaddr_in构造IPv4Address
     * @param address 
     */
    IPv4Address(const sockaddr_in& address);
    /**
     * @brief 通过二进制地址构造IPv4Address
     * @param address INADDR_ANY等于inet_addr("0.0.0.0")
     * @param port 
     */
    IPv4Address(uint32_t address = INADDR_ANY, uint16_t port = 0);
    const sockaddr* getAddr() const override;
    sockaddr* getAddr() override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;

    IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
    IPAddress::ptr networdAddress(uint32_t prefix_len) override;
    IPAddress::ptr subnetMask(uint32_t prefix_len) override;
    uint32_t getPort() const override;
    void setPort(uint16_t v) override;
private:
    sockaddr_in addr_;
};

class IPv6Address : public IPAddress
{
public:
    typedef std::shared_ptr<IPv6Address> ptr;
    /**
     * @brief 通过IPv6地址字符串构造IPV6Address 
     * @param address IPv6地址字符串
     * @param port 端口号
     * @return IPv6Address::ptr 
     */
    static IPv6Address::ptr Create(const char* address, uint16_t port = 0);
    /**
     * @brief Construct a new IPv6Address object
     */
    IPv6Address();
    /**
     * @brief Construct a new IPv6Address object
     * @param address 
     */
    IPv6Address(const sockaddr_in6& address);
    /**
     * @brief 通过IPV6二进制地址构造IPv6Address
     * @param address  IPv6二进制地址
     * @param port 
     */
    IPv6Address(const uint8_t address[16], uint16_t port = 0);
    const sockaddr* getAddr() const override;
    sockaddr* getAddr() override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;
    IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
    IPAddress::ptr networdAddress(uint32_t prefix_len) override;
    IPAddress::ptr subnetMask(uint32_t prefix_len) override;
    uint32_t getPort() const override;
    void setPort(uint16_t v) override;
private:
    sockaddr_in6 addr_;
};

class UnixAddress : public Address
{
public:
    typedef std::shared_ptr<UnixAddress> ptr;
    UnixAddress();
    /**
     * @brief Construct a new Unix Address object
     * @param path UnixSocket路径(长度小于UNIX_PATH_MAX)
     */
    UnixAddress(const std::string& path);
    const sockaddr* getAddr() const override;
    sockaddr* getAddr() override;
    socklen_t getAddrLen() const override;
    void setAddrLen(uint32_t v);
    std::string getPath() const;
    std::ostream& insert(std::ostream& os) const override;
private:
    sockaddr_un addr_;
    socklen_t length_;
};

class UnknownAddress : public Address
{
public:
    typedef std::shared_ptr<UnknownAddress> ptr;
    UnknownAddress(int family);
    UnknownAddress(const sockaddr& addr);
    const sockaddr* getAddr() const override;
    sockaddr* getAddr() override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;
private:
    sockaddr addr_;
};

std::ostream& operator<<(std::ostream& os, const Address& addr);

}
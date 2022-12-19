/*
 * @Descripttion: 
 * @version: 
 * @Author: leileilei
 * @Date: 2022-11-24 15:54:07
 * @LastEditors: sueRimn
 * @LastEditTime: 2022-12-19 15:23:33
 */
#include "address.h"
#include "log.h"
#include <sstream>
#include <netdb.h>
#include <ifaddrs.h>
#include <stddef.h>

#include "myendian.h"

namespace leileilei
{

static leileilei::Logger::ptr g_logger = LEI_GET_LOGGER("system") ;

// value二进制有多少个1 
template<class T>
static uint32_t CountBytes(T value)
{
    uint32_t result = 0;
    for(;value;result++)
    {
        value &= value - 1;
    }
    return result;
}

// 子网掩码的反码
template<class T>
static T CreateMask(uint32_t bits)
{
    return (1 << (sizeof(T) * 8 - bits)) - 1; 
}

Address::ptr Address::Create(const sockaddr* addr, socklen_t addrlen) 
{
    if(addr == nullptr)
    {
        LEI_LOG_DEBUG(g_logger)<< "param addr is nullptr";
        return nullptr;
    }
    Address::ptr result;
    switch(addr->sa_family)
    {
        case AF_INET:
            result.reset(new IPv4Address(*((const sockaddr_in*)addr)));
            break;
        case AF_INET6:
            result.reset(new IPv6Address(*((const sockaddr_in6*)addr)));
            break;
        default:
            result.reset(new UnknownAddress(*addr));
    }
    return result;
}

bool Address::Lookup(std::vector<Address::ptr>& result, const std::string& host, int family, int type, int protocol) 
{
    LEI_LOG_DEBUG(g_logger) << "come to Lookup";
    addrinfo hints, *results, *next;
    hints.ai_flags = 0;
    hints.ai_family = family;
    hints.ai_socktype = type;
    hints.ai_protocol = protocol;
    hints.ai_addrlen = 0;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    
    std::string node;
    const char* service = NULL;

    // 检查 ipv6address service
    if(!host.empty() && host[0] == '[')
    {
        /**
         * C 库函数 void *memchr(const void *str, int c, size_t n) 在参数 str 所指向的字符串的前 n 个字节中搜索第一次出现字符 c（一个无符号字符）的位置。
         */
        const char* endipv6 = (const char*)memchr(host.c_str()+1, ']', host.size() -1);
        if(endipv6)
        {
            if(*(endipv6+1) == ':')
            {
                service = endipv6 + 2;
            }
            node = host.substr(1, endipv6 - host.c_str() - 1);
        }
    }

    // 检查 node service
    if(node.empty())
    {
        service = (const char*)memchr(host.c_str(), ':', host.size());
        if(service)
        {
            if(!memchr(service+1, ':', host.c_str() + host.size() - service - 1))
            {
                node = host.substr(0, service - host.c_str());
                service++;
            }
        }
    }

    if(node.empty())
    {
        node = host;
    }

    LEI_LOG_DEBUG(g_logger) << "111111";
    int error = getaddrinfo(node.c_str(), service, &hints, &results);
    LEI_LOG_DEBUG(g_logger) << "22222";
    if(error)
    {
        LEI_LOG_ERROR(g_logger) << "Address::Lookup getaddress(" << host << ", "
            << family << ", " << type << ") err=" << error << " errstr="
            << gai_strerror(error);
        return false;
    }

    next = results;
    
    while(next)
    {
        result.push_back(Create(next->ai_addr, (socklen_t)next->ai_addrlen));
        next = next->ai_next;
    }
    freeaddrinfo(results);
    LEI_LOG_DEBUG(g_logger) << "Lookup end";
    return !result.empty();
}

Address::ptr Address::LookupAny(const std::string& host, int family, int type, int protocol) 
{
    std::vector<Address::ptr> result;
    if(Lookup(result, host, family, type, protocol))
    {
        return result[0];
    }
    LEI_LOG_DEBUG(g_logger) << "this host:"<< host <<" no available address";
    return nullptr;
}

IPAddress::ptr Address::LookupAnyIPAddress(const std::string& host, int family, int type, int protocol)  
{
    std::vector<Address::ptr> result;
    if(Lookup(result, host, family, type, protocol))
    {
        for(auto& v:result)
        {
            IPAddress::ptr ans = std::dynamic_pointer_cast<IPAddress>(v);
            if(ans)
                return ans;
        }
        LEI_LOG_DEBUG(g_logger) << "class Address transfer to IPAddress error";
    }
    LEI_LOG_DEBUG(g_logger) << "this host:"<< host <<" no available address";
    return nullptr;
}

bool Address::GetInterfaceAddresses(std::multimap<std::string, std::pair<Address::ptr, uint32_t> >& result, int family) 
{
    struct ifaddrs *next, *results;
    if(getifaddrs(&results) != 0)
    {
        LEI_LOG_ERROR(g_logger) << "Address::GetInterfaceAddresses getifaddrs "
            " err=" << errno << " errstr=" << strerror(errno);
        return false;
    }

    try
    {
        for(next = results; next; next = next->ifa_next)
        {
            Address::ptr addr;
            uint32_t prefix_len = ~0u;
            if(family != AF_UNSPEC && family != next->ifa_addr->sa_family)
                continue;
            
            switch(next->ifa_addr->sa_family)
            {
                case AF_INET:
                    {
                        addr = Create(next->ifa_addr, sizeof(sockaddr_in));
                        uint32_t netmask = ((sockaddr_in*)next->ifa_netmask)->sin_addr.s_addr;
                        prefix_len = CountBytes(netmask);
                    }
                    break;
                case AF_INET6:
                    {
                        addr = Create(next->ifa_addr, sizeof(sockaddr_in6));
                        in6_addr& netmask = ((sockaddr_in6*)next->ifa_netmask)->sin6_addr;
                        prefix_len = 0;
                        for(int i=0; i<16; i++)
                        {
                            prefix_len += CountBytes(netmask.s6_addr[i]);
                        }
                    }
                    break;
                default:
                    break;
            }

            if(addr)
            {
                result.insert(std::make_pair(next->ifa_name, std::make_pair(addr, prefix_len)));
            }
        }
    }
    catch(...)
    {
        LEI_LOG_ERROR(g_logger) << "Address::GetInterfaceAddresses exception";
        freeifaddrs(results);
        return false;
    }
    freeifaddrs(results);
    return !result.empty();
}

bool Address::GetInterfaceAddresses(std::vector<std::pair<Address::ptr, uint32_t> >& result, const std::string& iface, int family) 
{
    if(iface.empty() || iface == "*")
    {
        if(family == AF_INET || family == AF_UNSPEC)
            result.push_back(std::make_pair(Address::ptr(new IPv4Address()), 0u));
        if(family == AF_INET6 || family == AF_UNSPEC)
            result.push_back(std::make_pair(Address::ptr(new IPv6Address()), 0u));
        return true;
    }

    std::multimap<std::string, std::pair<Address::ptr, uint32_t>> results;
    
    if(!GetInterfaceAddresses(results, family))
        return false;
    
    auto its = results.equal_range(iface);
    for(;its.first != its.second; ++its.first)
    {
        result.push_back(its.first->second);
    }
    return !result.empty();
}

int Address::getFamily() const 
{
    return getAddr()->sa_family;
}

std::string Address::toString() const 
{
    std::stringstream ss;
    insert(ss);
    return ss.str();
}

bool Address::operator<(const Address& rhs) const 
{
    socklen_t minlen = std::min(getAddrLen(), rhs.getAddrLen());
    /**
        头  文  件：#include <string.h>
        函数声明：int memcmp ( const void * ptr1, const void * ptr2, size_t num );
        参数：
            ptr1：指向内存块的指针。
            ptr2：指向内存块的指针。
            num：要比较的字节数。
        返回值：
            <0:在两个内存块中不匹配的第一个字节在 ptr1 中的值低于在 ptr2 中的值（如果计算为无符号 char 值）
            0:两个内存块的内容相等
            >0:在两个内存块中不匹配的第一个字节在 ptr1 中的值大于在 ptr2 中的值（如果计算为无符号字符值）
     */
    int result = memcmp(getAddr(), rhs.getAddr(), minlen);
    if(result < 0)
        return true;
    else if(result > 0)
        return false;
    else if(getAddrLen() < rhs.getAddrLen())
        return true;
    return false;
}

bool Address::operator==(const Address& rhs) const 
{
    return getAddrLen() == rhs.getAddrLen()
            && memcmp(getAddr(), rhs.getAddr(), getAddrLen()) == 0;
}

bool Address::operator!=(const Address& rhs) const 
{
    return !(*this == rhs);
}

IPAddress::ptr IPAddress::Create(const char* address, uint16_t port)
{
    addrinfo hints, *results;
    memset(&hints, 0, sizeof(addrinfo));
    // AI_NUMERICHOST 传递给getaddrinfo函数的nodename参数必须是数字字符串
    hints.ai_flags = AI_NUMERICHOST;
    hints.ai_family = AF_UNSPEC;

    int error = getaddrinfo(address, NULL, &hints, &results);
    if(error)
    {
        LEI_LOG_ERROR(g_logger) << "IPAddress::Create(" << address
            << ", " << port << ") error=" << error
            << " errno=" << errno << " errstr=" << strerror(errno);
        return nullptr;
    }

    try
    {
        IPAddress::ptr result = std::dynamic_pointer_cast<IPAddress>(Address::Create(results->ai_addr, (socklen_t)results->ai_addrlen));
        if(result)
            result->setPort(port);
        freeaddrinfo(results);
        return result;
    }
    catch(...)
    {
        freeaddrinfo(results);
        return nullptr;
    }
}

IPv4Address::ptr IPv4Address::Create(const char* address, uint16_t port)
{
    IPv4Address::ptr rt(new IPv4Address);
    rt->addr_.sin_port  = byteswapOnLittleEndian(port);
    /**
    #include <arpe/inet.h>
    int inet_pton(int family, const char *strptr, void *addrptr);     //将点分十进制的ip地址转化为用于网络传输的数值格式
        返回值：若成功则为1，若输入不是有效的表达式则为0，若出错则为-1
 
    const char * inet_ntop(int family, const void *addrptr, char *strptr, size_t len);     //将数值格式转化为点分十进制的ip地址格式
        返回值：若成功则为指向结构的指针，若出错则为NULL
     */
    int result = inet_pton(AF_INET, address, &rt->addr_.sin_addr);
    if(result <= 0)
    {
        LEI_LOG_ERROR(g_logger) << "IPv4Address::Create(" << address << ", "
                << port << ") rt=" << result << " errno=" << errno
                << " errstr=" << strerror(errno);
        return nullptr;
    }
    return rt;
}
    
IPv4Address::IPv4Address(const sockaddr_in& address)
{
    addr_ = address;
}

IPv4Address::IPv4Address(uint32_t address, uint16_t port)
{
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = byteswapOnLittleEndian(port);
    addr_.sin_addr.s_addr = byteswapOnLittleEndian(address);
}

const sockaddr* IPv4Address::getAddr() const 
{
    return (sockaddr*)&addr_;
}

sockaddr* IPv4Address::getAddr() 
{
    return (sockaddr*)&addr_;
}

socklen_t IPv4Address::getAddrLen() const
{
    return sizeof(addr_);
}

std::ostream& IPv4Address::insert(std::ostream& os) const 
{
    uint32_t addr = byteswapOnLittleEndian(addr_.sin_addr.s_addr);
    // 0xff 为8个1   1111 1111
    os << ((addr>>24) & 0xff) << "."
        << ((addr>>16) & 0xff) << "."
        << ((addr>>8) & 0xff) << "."
        << (addr & 0xff);
    os << ":" << byteswapOnLittleEndian(addr_.sin_port);
    return os;
}

IPAddress::ptr IPv4Address::broadcastAddress(uint32_t prefix_len) 
{
    if(prefix_len > 32)
        return nullptr;
    
    sockaddr_in baddr(addr_);
    baddr.sin_addr.s_addr |= byteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));
    return IPAddress::ptr(new IPv4Address(baddr));
}

IPAddress::ptr IPv4Address::networdAddress(uint32_t prefix_len) 
{
    if(prefix_len > 32)
        return nullptr;
    sockaddr_in baddr(addr_);
    baddr.sin_addr.s_addr &= byteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));
    return IPv4Address::ptr(new IPv4Address(baddr));
}

IPAddress::ptr IPv4Address::subnetMask(uint32_t prefix_len) 
{   
    if(prefix_len > 32)
        return nullptr;
    sockaddr_in subnet;
    memset(&subnet, 0 , sizeof(subnet));
    subnet.sin_family = AF_INET;
    subnet.sin_addr.s_addr = ~byteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));
    return IPv4Address::ptr(new IPv4Address(subnet));
}

uint32_t IPv4Address::getPort() const 
{
    return byteswapOnLittleEndian(addr_.sin_port);
}

void IPv4Address::setPort(uint16_t v) 
{
    addr_.sin_port = byteswapOnLittleEndian(v);
}

IPv6Address::ptr IPv6Address::Create(const char* address, uint16_t port)
{
    IPv6Address::ptr rt(new IPv6Address);
    rt->addr_.sin6_port = byteswapOnLittleEndian(port);
    int result = inet_pton(AF_INET6, address, &rt->addr_.sin6_addr);
    if(result <= 0)
    {
        LEI_LOG_ERROR(g_logger) << "IPv6Address::Create(" << address << ", "
                << port << ") rt=" << result << " errno=" << errno
                << " errstr=" << strerror(errno);
        return nullptr;
    }
    return rt;
}

IPv6Address::IPv6Address()
{
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin6_family = AF_INET6;
}

IPv6Address::IPv6Address(const sockaddr_in6& address)
{
    addr_ = address;
}

IPv6Address::IPv6Address(const uint8_t address[16], uint16_t port)
{
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin6_family = AF_INET6;
    addr_.sin6_port = byteswapOnLittleEndian(port);
    memcmp(&addr_.sin6_addr.s6_addr, address, 16);
}

const sockaddr* IPv6Address::getAddr() const
{
    return (sockaddr*)&addr_;
}

sockaddr* IPv6Address::getAddr() 
{
    return (sockaddr*)&addr_;
}

socklen_t IPv6Address::getAddrLen() const
{
    return sizeof(addr_);
}

std::ostream& IPv6Address::insert(std::ostream& os) const
{
    os << "[";
    uint16_t* addr = (uint16_t*)addr_.sin6_addr.s6_addr;
    bool used_zeros = false;
    for(size_t i=0;i<8;i++)
    {
        if(addr[i] == 0 && !used_zeros)
            continue;
        if(i && addr[i-1] == 0 && !used_zeros)
        {
            os << ":";
            used_zeros = true;
        }
        if(i)
            os << ":";
        os << std::hex << (int)byteswapOnLittleEndian(addr[i]) << std::dec;
    }

    if(!used_zeros && addr[7] == 0)
        os << "::";
    
    os << "]:" << byteswapOnLittleEndian(addr_.sin6_port);
    return os;
}

IPAddress::ptr IPv6Address::broadcastAddress(uint32_t prefix_len)
{
    sockaddr_in6 baddr(addr_);
    baddr.sin6_addr.s6_addr[prefix_len / 8] &= CreateMask<uint8_t>(prefix_len % 8);
    for(int i=prefix_len/8 + 1; i<16; i++)
    {
        baddr.sin6_addr.s6_addr[i] = 0xff;
    }
    return IPv6Address::ptr(new IPv6Address(baddr));
}

IPAddress::ptr IPv6Address::networdAddress(uint32_t prefix_len) 
{
    sockaddr_in6 baddr(addr_);
    baddr.sin6_addr.s6_addr[prefix_len / 8] &=
        CreateMask<uint8_t>(prefix_len % 8);
    for(int i = prefix_len / 8 + 1; i < 16; ++i) {
        baddr.sin6_addr.s6_addr[i] = 0x00;
    }
    return IPv6Address::ptr(new IPv6Address(baddr));
}

IPAddress::ptr IPv6Address::subnetMask(uint32_t prefix_len)
{
    sockaddr_in6 subnet;
    memset(&subnet, 0, sizeof(subnet));
    subnet.sin6_family = AF_INET6;
    subnet.sin6_addr.s6_addr[prefix_len /8] =
        ~CreateMask<uint8_t>(prefix_len % 8);

    for(uint32_t i = 0; i < prefix_len / 8; ++i) {
        subnet.sin6_addr.s6_addr[i] = 0xff;
    }
    return IPv6Address::ptr(new IPv6Address(subnet));
}

uint32_t IPv6Address::getPort() const 
{
    return byteswapOnLittleEndian(addr_.sin6_port);
}

void IPv6Address::setPort(uint16_t v)
{
    addr_.sin6_port = byteswapOnLittleEndian(v);
}

static const size_t MAX_PATH_LEN = sizeof(((sockaddr_un*)0)->sun_path) - 1;

UnixAddress::UnixAddress()
{
    memset(&addr_, 0, sizeof(addr_));
    addr_.sun_family = AF_UNIX;
    /**
        #include <stddef.h>
        #include <stdio.h>

        struct address {
            char name[50];
            char street[50];
            int phone;
        };
        
        int main()
        {
            printf("address 结构中的 name 偏移 = %d 字节。\n",
            offsetof(struct address, name));
            
            printf("address 结构中的 street 偏移 = %d 字节。\n",
            offsetof(struct address, street));
            
            printf("address 结构中的 phone 偏移 = %d 字节。\n",
            offsetof(struct address, phone));

            return(0);
        } 
        编译并运行上面的程序，这将产生以下结果：
        address 结构中的 name 偏移 = 0 字节。
        address 结构中的 street 偏移 = 50 字节。
        address 结构中的 phone 偏移 = 100 字节。
     */
    length_ = offsetof(sockaddr_un, sun_path) + MAX_PATH_LEN;
}

UnixAddress::UnixAddress(const std::string& path)
{
    memset(&addr_, 0, sizeof(addr_));
    addr_.sun_family = AF_UNIX;
    length_ = path.size() + 1;

    if(!path.empty() && path[0] == '\0')
    {
        length_--;
    }

    if(length_ > sizeof(addr_.sun_path))
        throw std::logic_error("path too long");

    memcmp(addr_.sun_path, path.c_str(), length_);
    length_ += offsetof(sockaddr_un, sun_path);
}

const sockaddr* UnixAddress::getAddr() const
{
    return (sockaddr*)&addr_;    
}

sockaddr* UnixAddress::getAddr()
{
    return (sockaddr*)&addr_;
}

socklen_t UnixAddress::getAddrLen() const
{
    return length_;
}

void UnixAddress::setAddrLen(uint32_t v)
{
    length_ = v;
}
std::string UnixAddress::getPath() const
{
    std::stringstream ss;
    if(length_ > offsetof(sockaddr_un, sun_path) && addr_.sun_path[0] == '\0')
        ss << "\\0" << std::string(addr_.sun_path+1, length_ - offsetof(sockaddr_un, sun_path)-1);
    else
        ss << addr_.sun_path;
    
    return ss.str();
}

std::ostream& UnixAddress::insert(std::ostream& os) const
{
    if(length_ > offsetof(sockaddr_un, sun_path) && addr_.sun_path[0] == '\0')
        return os << "\\0" << std::string(addr_.sun_path+1, length_ - offsetof(sockaddr_un, sun_path)-1);
    return os << addr_.sun_path;
}

UnknownAddress::UnknownAddress(int family)
{
    memset(&addr_, 0, sizeof(addr_));
    addr_.sa_family = family;
}

UnknownAddress::UnknownAddress(const sockaddr& addr)
{
    addr_ = addr;
}

const sockaddr* UnknownAddress::getAddr() const 
{
    return (sockaddr*)&addr_;
}

sockaddr* UnknownAddress::getAddr() 
{
    return (sockaddr*)&addr_;
}

socklen_t UnknownAddress::getAddrLen() const
{
    return sizeof(addr_);
}

std::ostream& UnknownAddress::insert(std::ostream& os) const 
{
    os << "[UnknownAddress family = " << addr_.sa_family << "]";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Address& addr)
{
    return addr.insert(os);
}


}
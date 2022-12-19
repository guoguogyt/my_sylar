/*
 * @Descripttion: 
 * @version: 
 * @Author: leileilei
 * @Date: 2022-12-15 10:17:02
 * @LastEditors: sueRimn
 * @LastEditTime: 2022-12-19 09:20:39
 */
#pragma once

#include <memory>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include "nonecopy.h"
#include "address.h"

namespace leileilei
{
    
class Socket : public std::enable_shared_from_this<Socket>, NoneCopy
{
public:
    typedef std::shared_ptr<Socket> ptr;
    typedef std::weak_ptr<Socket> weak_ptr;

    /**
     * @brief Socket类型 
     */
    enum Type
    {
        TCP = SOCK_STREAM,
        UDP = SOCK_DGRAM
    };

    /**
     * @brief Socket协议簇
     */
    enum Family
    {
      IPv4 = AF_INET,
      IPv6 = AF_INET6,
      UNIX = AF_UNIX,  
    };

    /**
     * @brief 创建TCP Socket(满足地址类型)
     * @param address 地址
     * @return Socket::ptr 
     */
    static Socket::ptr CreateTCP(leileilei::Address::ptr address);

    /**
     * @brief 创建UDP Socket(满足地址类型)
     * @param address 地址
     * @return Socket::ptr 
     */
    static Socket::ptr CreateUDP(leileilei::Address::ptr address);

    /**
     * @brief 创建IPv4的TCP Socket
     * @return Socket::ptr 
     */
    static Socket::ptr CreateTCPIPv4Socket();

    /**
     * @brief 创建IPv4的UDP Socket
     * @return Socket::ptr 
     */
    static Socket::ptr CreateUDPIPv4Socket();

    /**
     * @brief 创建IPv6的TCP Socket
     * @return Socket::ptr 
     */
    static Socket::ptr CreateTCPIPv6Socket();
    /**
     * @brief 创建IPv6的UDP Socket 
     * @return Socket::ptr 
     */
    static Socket::ptr CreateUDPIPv6Socket();

    /**
     * @brief 创建unix的TCP Socket
     * @return Socket::ptr 
     */
    static Socket::ptr CreateUnixTCPSocket();
    /**
     * @brief 创建unix的UDP Socket
     * @return Socket::ptr 
     */
    static Socket::ptr CreateUnixUDPSocket();

    /**
     * @brief Socket的构造函数
     * @param family 协议簇
     * @param type 类型
     * @param protocol 协议
     */
    Socket(int family, int type, int protocol = 0);

    /**
     * @brief 析构函数
     */
    virtual ~Socket();

    /**
     * @brief 获取发送超时时间(毫秒)
     * @return int64_t 
     */
    int64_t getSendTimeout();
    /**
     * @brief 设置发送超时时间(毫秒)
     */
    void setSendTimeout();

    /**
     * @brief 获取接收超时时间(毫秒)
     * @return int64_t 
     */
    int64_t getRecvTimeout();
    /**
     * @brief 设置接收超时时间(毫秒)
     */
    void setRecvTimeout();

    /**
     * @brief 获取socket    参照getsockopt
     * @param level 
     * @param option 
     * @param result 
     * @param len 
     * @return true 
     * @return false 
     */
    bool getOption(int level, int option, void* result, socklen_t* len);
    /**
     * @brief Get the Option object
     * @tparam T 
     * @param level 
     * @param option 
     * @param result 
     * @return true 
     * @return false 
     */
    template<class T>
    bool getOption(int level, int option, T& result)
    {
        socklen_t length = sizeof(T);
        return getOption(level, option, &result, length);
    }

    /**
     * @brief 设置sockopt @see setsockopt
     * @param level 
     * @param option 
     * @param result 
     * @param len 
     * @return true 
     * @return false 
     */
    bool setOption(int level, int option, const void* result, socklen_t len);
    /**
     * @brief Set the Option object
     * @tparam T 
     * @param level 
     * @param option 
     * @param result 
     * @return true 
     * @return false 
     */
    template<class T>
    bool setOption(int level, int option, const T& result)
    {
        return setOption(level, option, &result, sizeof(T));
    }

    /**
     * @brief 绑定地址
     * @param addr 
     * @return true 
     * @return false 
     */
    virtual bool bind(const Address::ptr addr);

    /**
     * @brief 监听socket
     * @param backlog 未完成连接队列的最大长度
     * @return true 
     * @return false 
     */
    virtual bool listen(int backlog = SOMAXCONN);

    /**
     * @brief 接收connect连接 
     * @return 成功返回新链接的socket ， 失败则返回nullptr
     * @pre Socket必须 bind listen成功
     */
    virtual Socket::ptr accept();

    /**
     * @brief 连接地址
     * @param addr 目标地址
     * @param timeout_ms 超时时间(毫秒)
     * @return true 
     * @return false 
     */
    virtual bool connect(const Address::ptr addr, uint64_t timeout_ms = -1);

    virtual bool reconnect(uint64_t timeout_ms = -1);

    /**
     * @brief 关闭socket 
     * @return true 
     * @return false 
     */
    virtual bool close();

    /**
     * @brief 发送数据 
     * @param buffer 待发送数据的内存
     * @param length 待发送数据的长度
     * @param flags 标志字
     * @return int 
        >0 发送成功对应大小的数据
        =0 socket被关闭
        <0 socket出错
     */
    virtual int send(const void* buffer, size_t length, int flags = 0);
    /**
     * @brief 发送数据
     * @param buffer 待发送数据的内存(iovec数组)
     * @param length 待发送数据的长度(iovec长度)
     * @param flags 标志字
     * @return int 
        >0 发送成功对应大小的数据
        =0 socket被关闭
        <0 socket出错
     */
    virtual int send(const iovec* buffer, size_t length, int flags = 0);
    
    /**
     * @brief 发送数据
     * @param buffer 待发送数据的内存
     * @param length 待发送数据的长度
     * @param to 发送的目标地址
     * @param flags 标志字
     * @return int 
        >0 发送成功对应大小的数据
        =0 socket被关闭
        <0 socket出错
     */
    virtual int sendTo(const void* buffer, size_t length, const Address::ptr to, int flags = 0);

    /**
     * @brief  发送数据
     * @param buffer 待发送数据的内存
     * @param length 待发送数据的长度
     * @param to 发送的目标地址
     * @param flags 标志字
     * @return int 
        >0 发送成功对应大小的数据
        =0 socket被关闭
        <0 socket出错
     */
    virtual int sendTo(const iovec* buffer, size_t length, const Address::ptr to, int flags = 0);

    /**
     * @brief 接受数据 
     * @param buffer 接受数据的内存
     * @param length 接受数据的内存大小
     * @param flags 标志字
     * @return int 
        >0 发送成功对应大小的数据
        =0 socket被关闭
        <0 socket出错
     */
    virtual int recv(void* buffer, size_t length, int flags = 0);
    /**
     * @brief 接受数据 
     * @param buffer 接受数据的内存(iovec数组)
     * @param length 接受数据的内存大小
     * @param flags 标志字
     * @return int 
        >0 发送成功对应大小的数据
        =0 socket被关闭
        <0 socket出错
     */
    virtual int recv(iovec* buffer, size_t length, int flags = 0);
    /**
     * @brief 接受数据 
     * @param buffer 接受数据的内存
     * @param length 接受数据的大小
     * @param from 发送端的地址
     * @param flags 标志字
     * @return int 
        >0 发送成功对应大小的数据
        =0 socket被关闭
        <0 socket出错
     */
    virtual int recvFrom(void* buffer, size_t length, Address::ptr from, int flags = 0);
    /**
     * @brief 接受数据
     * @param buffer 接受数据的内存(iovec数组)
     * @param length 接受数据的大小
     * @param from 发送端的地址
     * @param flags 标志字
     * @return int 
        >0 发送成功对应大小的数据
        =0 socket被关闭
        <0 socket出错
     */
    virtual int recvFrom(iovec* buffer, size_t length, Address::ptr from, int flags = 0);

    /**
     * @brief 获取远端地址
     * @return Address::ptr 
     */
    Address::ptr getRemoteAddress();
    /**
     * @brief 获取本地地址 
     * @return Address::ptr 
     */
    Address::ptr getlocalAddress();
    /**
     * @brief 获取协议簇
     * @return int 
     */
    int getFamily() const {return family_;}
    /**
     * @brief 获取类型
     * @return int 
     */
    int getType() const {return type_;}
    /**
     * @brief 获取协议
     * @return int 
     */
    int getProtocol() const {return protocol_;}
    /**
     * @brief 获取是否连接 
     * @return true 
     * @return false 
     */
    bool isConnected() const {return isConnect_;}
    /**
     * @brief socket是否有效(sock_ != -1) 
     * @return true 
     * @return false 
     */
    bool isValid() const;
    /**
     * @brief 返回Socket错误
     * @return int 
     */
    int getError();
    /**
     * @brief 输出信息到流中
     * @param os 
     * @return std::ostream& 
     */
    virtual std::ostream& dump(std::ostream& os) const;
    virtual std::string toString() const;
    /**
     * @brief Get the Socket object
     * @return int 
     */
    int getSocket() const {return sock_;}
    /**
     * @brief 取消读 
     * @return true 
     * @return false 
     */
    bool cancelRead();
    /**
     * @brief 取消写 
     * @return true 
     * @return false 
     */
    bool cancelWrite();
    /**
     * @brief 取消accept 
     * @return true 
     * @return false 
     */
    bool cancelAccept();
    /**
     * @brief 取消所有事件
     * @return true 
     * @return false 
     */
    bool cancelAll();

protected:
    /**
     * @brief 初始化socket 
     */
    void initSock();
    /**
     * @brief 创建socket 
     */
    void newSock();
    /**
     * @brief 初始化sock
     * @param sock 
     * @return true 
     * @return false 
     */
    virtual bool init(int sock);
private:
    // socket句柄 
    int sock_;
    // 协议簇
    int family_;
    // 类型
    int type_;
    // 协议
    int protocol_;
    // 是否连接
    bool isConnect_;
    // 本地地址
    Address::ptr localAddress_;
    // 远端地址
    Address::ptr remoteAddress_;
};

}
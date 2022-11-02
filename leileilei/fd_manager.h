/*
 * @Descripttion: 
 * @version: 
 * @Author: leileilei
 * @Date: 2022-11-02 14:13:28
 * @LastEditors: sueRimn
 * @LastEditTime: 2022-11-02 15:18:09
 */
#pragma once

#include <memory>
#include <vector>
#include "mutex.h"
#include "singleton.h"

namespace leileilei
{

/**
 * @brief 
 * 封装fd
 */
class FdCtx : public std::enable_shared_from_this<FdCtx>    
{
public:
    typedef std::shared_ptr<FdCtx> ptr;

    FdCtx(int fd);
    ~FdCtx();
    bool isInit() const {   return isInit_;}
    bool isSocket() const { return isSocket_;}
    bool isClosed() const { return isClosed_;}
    void setUserNoblock(bool flag) {    userNoblock_ = flag;}
    bool getUserNoblock() const {   return userNoblock_;}
    void setSysNoblock(bool flag)   {   sysNoblock_ = flag;}
    bool getSysNoblock() const {    return sysNoblock_;}
    void setTimeout(int type, uint64_t ms);
    uint64_t getTimeout(int type);
private:
    bool init();
private:
    // fd是否初始化
    bool isInit_;
    // fd是否是socket
    bool isSocket_;
    // fd是否非阻塞
    bool sysNoblock_;
    // fd是否用户主动设置阻塞
    bool userNoblock_;
    // fd是否已经关闭
    bool isClosed_;
    // fd
    int fd_;
    // 读超时时间 ms
    uint64_t recvTimeout_;
    // 写超时时间 ms
    uint64_t sendTimeout_;
};

/**
 * @brief 
 * 管理fd
 */
class FdManager
{
public:
    typedef RWMutex RWMutexType;

    FdManager();
    /**
     * @brief 
     *  获取fd， 如果不存在可以由参数auto_create决定是否创建
     * @param fd 
     * @param auto_create 
     * @return FdCtx::ptr 
     */
    FdCtx::ptr get(int fd, bool auto_create);
    /**
     * @brief 
     *  删除某个fd
     * @param fd 
     */
    void del(int fd);
private:
    RWMutexType mutex_;
    // fd队列
    std::vector<FdCtx::ptr> fd_list_;
};

typedef Singleton<FdManager>  FdMgr;

}
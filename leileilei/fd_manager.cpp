/*
 * @Descripttion: 
 * @version: 
 * @Author: leileilei
 * @Date: 2022-11-02 14:13:37
 * @LastEditors: sueRimn
 * @LastEditTime: 2022-11-02 15:06:17
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "fd_manager.h"

namespace leileilei
{
FdCtx::FdCtx(int fd):
    isInit_(false),
    isSocket_(false),
    sysNoblock_(false),
    userNoblock_(false),
    isClosed_(false),
    fd_(fd),
    recvTimeout_(-1),
    sendTimeout_(-1)
{
    init();
}

FdCtx::~FdCtx()
{}

void FdCtx::setTimeout(int type, uint64_t ms)
{
    if(type == SO_RCVTIMEO)
        recvTimeout_ = ms;
    else
        sendTimeout_ = ms;
}

uint64_t FdCtx::getTimeout(int type)
{
    if(type == SO_RCVTIMEO)
        return recvTimeout_;
    else
        return sendTimeout_;        
} 

bool FdCtx::init()
{
    if(isInit_)
    {
        return true;
    }
    struct stat fd_stat;
    if(fstat(fd_, fd_stat) == -1)
    {
        isInit_ = false;
        isSocket_ = false;
    }
    else
    {
        isInit_ = true;
        isSocket_ = S_ISSOCK(fd_stat.st_mode);
    }
    // 是否是socket, 如果是socket则把它变为非阻塞
    if(isSocket_)
    {
        int flags = fcntl_f(fd_, F_GETFL, 0);
        if(!(flags & O_NONBLOCK))
        {
            fcntl_f(fd_, F_SETFL, flags | O_NONBLOCK);
        }
        sysNoblock_ = true;
    }
    return isInit_;
}

FdManager::FdManager()
{
    fd_list_.resize(64);
}

FdCtx::ptr FdManager::get(int fd, bool auto_create)
{
    if(fd == -1)
        return nullptr;
    RWMutexType::ReadLock lock(mutex_);
    if((int)fd_list_.size() < fd)
    {
        if(!auto_create)
            return nullptr;
    }
    else
    {
        if(fd_list_[fd] || !auto_create)
            return fd_list_[fd];
    }
    lock.unlock();

    RWMutexType::WriteLock lock2(mutex_);
    FdCtx::ptr ctx(new FdCtx(fd));
    if((int)fd_list_.size() <= fd)
        fd_list_.resize(fd * 1.5);
    fd_list_[fd] = ctx;
    return ctx;
}

void FdManager::del(int fd)
{
    RWMutexType::WriteLock lock(mutex_);
    if((int)fd_list_.size() <= fd)
        return;
    fd_list_[fd].reset();
}

}
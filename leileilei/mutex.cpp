#include "mutex.h"

namespace leileilei
{

Semaphore::Semaphore(uint32_t count)
{   
    /**
     * @brief 
     * sem_init
        调用函数所需头文件：semaphore.h
        函数原型 ：int sem_init(sem_t *sem, int pshared, unsigned int value);
        参数解释 ：
        sem ：指向信号量对象
        pshared : 指明信号量的类型。若是 pshared 的值为 0，那么 信号量将被进程内的 线程共享，而且应该放置在这个进程的全部线程均可见的地址上(如 全局变量，或者堆上动态分配的变量)。
`                                 若是 pshared 是非零值，那么 信号量将在进程之间共享，而且应该定位 共享内存区域(见 shm_open(3)、mmap(2) 和 shmget(2))
        value : 指定信号量值的大小
        返回值：成功返回0，失败时返回-1，并设置errno。
        作用：创建信号量，并为信号量值赋初值。
     */
    if(sem_init(&semaphore_, 0, count))
    {
        throw std::logic_error("semaphore init error");
    }
}

Semaphore::~Semaphore()
{
    /**
     * @brief Construct a new sem destory object
     sem_destroy
        调用函数所需头文件：semaphore.h
        函数原型：sem_destroy(sem_t  *sem);
        参数解释 ：
        sem ：指向信号量对象
        返回值：成功返回0，失败时返回-1，并设置errno
        作用: 清理信号量占有的资源，当调用该函数，而有线程等待此信号量时，将会返回错信息。
     * 
     */
    if(sem_destroy(&semaphore_))
    {
        throw std::logic_error("sem destory error");
    }
}

void Semaphore::wait()
{
    /**
     * @brief Construct a new if object
     sem_wait
        调用函数所需头文件：semaphore.h
        函数原型：sem_wait(sem_t  *sem);
        参数解释 ：
        sem ：指向信号量对象
        返回值：成功返回0，失败时返回-1，并设置errno
        作用: 以 阻塞 的方式等待信号量，当信号量的值大于零时，执行该函数信号量减一，当信号量为零时，调用该函数的线程将会阻塞。
     * 
     */
    if(sem_wait(&semaphore_))
    {
        throw std::logic_error("semaphore wait error");
    }
}
 void Semaphore::notify()
{   
    /**
     * @brief Construct a new if object
     sem_post
        调用函数所需头文件：semaphore.h
        函数原型：int sem_post(sem_t  *sem);
        参数解释 ：
        sem ：指向信号量对象
        返回值：成功返回0，失败时返回-1，并设置errno。
        作用: 以原子操作的方式为将信号量增加1
     * 
     */
    if(sem_post(&semaphore_))
    {
        throw std::logic_error("semaphore notify error");
    }
}
}
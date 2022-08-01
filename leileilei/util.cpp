#include "util.h"

namespace leileilei
{

pid_t GetThreadId()
{
/**
* 作用： syscall() 执行一个系统调用，根据指定的参数number和所有系统调用的汇编语言接口来确定调用哪个系统调用。
*       系统调用所使用的符号常量可以在头文件里面找到。
* 参数： number是系统调用号，number后面应顺序接上该系统调用的所有参数
    
    int syscall(int number, ...); 
*/
    return syscall(SYS_gettid);
}


}
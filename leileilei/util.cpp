/*
 * @Descripttion: 
 * @version: 
 * @Author: leileilei
 * @Date: 2022-08-01 11:14:15
 * @LastEditors: sueRimn
 * @LastEditTime: 2022-09-07 17:32:12
 */
#include "util.h"

namespace leileilei
{

static leileilei::Logger::ptr g_logger = LEI_GET_LOGGER("system");

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

// 统一名称
static std::string demangle(const char* str)
{
    size_t size = 0;
    int status = 0;
    std::string rt;
    rt.resize(256);
    /**
        使用backtrace()获取到的堆栈信息格式如下：
            0x401c43
            0x4024da
            0x402568
            0x402568
        使用backtrace_symbols()转换后的堆栈信息格式如下：
            ./all(_Z9BacktraceiiRKSs+0x58) [0x401c43]
            ./all(_Z3funv+0x48) [0x4024da]
            ./all(_Z3funv+0xd6) [0x402568]
            ./all(_Z3funv+0xd6) [0x402568]
            ./all(_Z3funv+0xd6) [0x402568]
        提取括号里面的字母,提取可以用如下代码实现:
            std::string sstr;
            sstr.resize(256);
            sscanf(str, "%*[^(]%*[^_]%255[^)+]", &sstr[0]);
     */
    if(1 == sscanf(str,  "%*[^(]%*[^_]%255[^)+]", &rt[0]))
    {
        char* v = abi::__cxa_demangle(&rt[0], nullptr, &size, &status);
        if(v) {
            std::string result(v);
            free(v);
            return result;
        }
    }
    if(1 == sscanf(str, "%255s", &rt[0])) {
        return rt;
    }
    return str;
}


void Backtrace(std::vector<std::string>& bt, int size = 64, int skip = 1)
{
    // 因为本系统使用的是协程技术栈，所以需要节约栈空间，利用malloc将数据创建在堆上
    void** array = (void**)malloc(sizeof(void*) * size);
    /**
     * @brief backtrace 回溯
     *  #include <execinfo.h>
        int backtrace(void **buffer, int size);
        
        功能描述：
            backtrace函数会将当前程序的调用堆栈信息写入buffer所指向的数组。
            buffer中的每一项都是void *类型的，是对应堆栈帧的返回地址。
            而size参数指定可以存储在缓冲区中的最大地址数，如果回溯大于size，则返回最近size个调用堆栈信息，为了获得完整的回溯，我们必须确保缓冲区和大小足够大。
            （需要注意的是返回信息是 倒序 的，最近的函数调用在最前面，最远的调用在返回信息的最后面）
        
        返回值：
            返回获取到的调用堆栈信息的数量，该值不大于size。
            如果返回值小于size，则表示获取到了存储调用堆栈信息；如果它等于size，那么它可能已经被截断了，在这种情况下，最早的堆栈帧的地址可能不会被返回了
     */
    size_t s = ::backtrace(array, size);

    /**
     * @brief #include <execinfo.h>
              char **backtrace_symbols(void *const *buffer, int size);
        功能描述：
            第一个参数是backtrace返回信息buffer，backtrace_symbols的功能将地址信息转换为一个字符串数组，用于描述堆栈信息。size参数指定缓冲区中地址的数量。
        返回值：
            backtrace_symbols的返回值是一个指向 字符串数组的指针，它的大小通buffer相同，每个字符串包含了一个相对于buffer中对应元素的可打印信息，
            它由函数名(如果可以确定)、函数的十六进制偏移量和实际返回地址(十六进制)组成。
            （需要注意的是backtrace_symbols的返回值是调用malloc申请的内存空间，调用者必须手动释放它，但是指针数组所指向的字符串不需要也不应该被释放）

     */
    char** strings = backtrace_symbols(array, s);
    if(strings == NULL)
    {
        free(strings);//释放堆内存
        free(array);
        LEI_LOG_ERROR(g_logger)<< "backtrace_synbols error";
        return;
    }

    for(size_t i=skip; i<s; i++)
    {
        bt.push_back(demangle(strings[i]));
    }

    free(strings);//释放堆内存
    free(array);
}

std::string BacktraceToString(int size = 64, int skip = 2, const std::string& prefix = " ")
{
    std::vector<std::string> bt;
    Backtrace(bt, size, skip);
    std::stringstream ss;
    for(auto str : bt)
    {
        ss<< prefix << str <<std::endl;
    }
    return ss.str();
}


}
/*
 * @Descripttion: 
 * @version: 
 * @Author: leileilei
 * @Date: 2022-09-08 11:11:38
 * @LastEditors: sueRimn
 * @LastEditTime: 2022-09-08 13:36:36
 */
// #include <assert.h>
#include "leileilei.h"

void fun()
{
    LEILEILEI_ASSERT(0);
}

int main(int argc, char* argv[])
{
    fun(1);
    return 0;
}
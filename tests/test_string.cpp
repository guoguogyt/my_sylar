/*
 * @Descripttion: 
 * @version: 
 * @Author: leileilei
 * @Date: 2022-12-30 13:49:51
 * @LastEditors: sueRimn
 * @LastEditTime: 2022-12-30 15:20:14
 */

#include "leileilei.h"

static leileilei::Logger::ptr g_logger = LEI_GET_LOGGER("system");

void test()
{
    int a = 1747;
    float b = 1.123;
    double c = 10.23402;

    std::string s1 = "4221";
    std::string s2 = "3.213";
    std::string s3 = "10.12031";

    LEI_LOG_DEBUG(g_logger) << "IntToString--" << IntToString(a);
    LEI_LOG_DEBUG(g_logger) << "StringToInt--" << StringToInt(b);
    LEI_LOG_DEBUG(g_logger) << "FloatToString--" << FloatToString(c);
    LEI_LOG_DEBUG(g_logger) << "StringToFloat--" << StringToFloat(s1);
    LEI_LOG_DEBUG(g_logger) << "DoubleToString--" << DoubleToString(s2);
    LEI_LOG_DEBUG(g_logger) << "StringToDouble--" << StringToDouble(s3);
}

int main(int argc, char* argv[])
{
    test();
    return 0;
}
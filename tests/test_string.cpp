/*
 * @Descripttion: 
 * @version: 
 * @Author: leileilei
 * @Date: 2022-12-30 13:49:51
 * @LastEditors: sueRimn
 * @LastEditTime: 2023-01-05 10:41:43
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

    LEI_LOG_DEBUG(g_logger) << "IntToString--" << leileilei::IntToString(a);
    LEI_LOG_DEBUG(g_logger) << "StringToInt--" << leileilei::StringToInt(s1);
    LEI_LOG_DEBUG(g_logger) << "FloatToString--" << leileilei::FloatToString(b);
    LEI_LOG_DEBUG(g_logger) << "StringToFloat--" << leileilei::StringToFloat(s2);
    LEI_LOG_DEBUG(g_logger) << "DoubleToString--" << leileilei::DoubleToString(c);
    LEI_LOG_DEBUG(g_logger) << "StringToDouble--" << leileilei::StringToDouble(s3);

    LEI_LOG_DEBUG(g_logger) << "FormatFouble--" << leileilei::FormatFouble(c,2);

    std::string s4 = "Hello Word! \n find something   ";
    std::vector<std::string> vec;
    leileilei::SplitString(vec, s4, " ");

    std::string s5 = "    4 index";
    leileilei::TrimLeft(s5);
    LEI_LOG_DEBUG(g_logger) << "TrimLeft--" << s5;

    std::string s6 = "   glkl     ";
    leileilei::TrimRight(s6);
    LEI_LOG_DEBUG(g_logger) << "TrimRight--" << s6 << "---end,begin=" << index+1;

    std::string s7 = "      ";
    leileilei::TrimRight(s7);
    LEI_LOG_DEBUG(g_logger) << "TrimRight--" << s7 << "---end,begin=" << index+1;

    std::string s8 = "    zhong         ";
    leileilei::TrimAll(s8);
    LEI_LOG_DEBUG(g_logger) << "TrimAll--" << s8 << "---end";
}

int main(int argc, char* argv[])
{
    test();
    return 0;
}
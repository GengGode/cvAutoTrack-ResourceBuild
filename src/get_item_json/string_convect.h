#pragma once
#include <string>
#include <windows.h>
inline std::string utf8_to_gbk(const std::string &utf8_string)
{
    std::string ret_string;
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8_string.c_str(), -1, NULL, 0);
    wchar_t *gbk_wstring = new wchar_t[len + 1];
    memset(gbk_wstring, 0, len * 2 + 2);
    MultiByteToWideChar(CP_UTF8, 0, utf8_string.c_str(), -1, gbk_wstring, len);
    len = WideCharToMultiByte(CP_ACP, 0, gbk_wstring, -1, NULL, 0, NULL, NULL);
    char *gbk_string = new char[len + 1];
    memset(gbk_string, 0, len + 1);
    WideCharToMultiByte(CP_ACP, 0, gbk_wstring, -1, gbk_string, len, NULL, NULL);
    ret_string = gbk_string;
    delete[] gbk_string;
    delete[] gbk_wstring;
    return ret_string;
}

inline std::string gbk_to_utf8(const std::string &gbk_string)
{
    std::string ret_string;
    int len = MultiByteToWideChar(CP_ACP, 0, gbk_string.c_str(), -1, NULL, 0);
    wchar_t *utf8_wstring = new wchar_t[len + 1];
    memset(utf8_wstring, 0, len * 2 + 2);
    MultiByteToWideChar(CP_ACP, 0, gbk_string.c_str(), -1, utf8_wstring, len);
    len = WideCharToMultiByte(CP_UTF8, 0, utf8_wstring, -1, NULL, 0, NULL, NULL);
    char *utf8_string = new char[len + 1];
    memset(utf8_string, 0, len + 1);
    WideCharToMultiByte(CP_UTF8, 0, utf8_wstring, -1, utf8_string, len, NULL, NULL);
    ret_string = utf8_string;
    delete[] utf8_string;
    delete[] utf8_wstring;
    return ret_string;
}
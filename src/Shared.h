#ifndef VTPBUDDY_SHARED_H
#define VTPBUDDY_SHARED_H

#include <algorithm>

inline std::string &str_trim_left(std::string &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

inline std::string &str_trim_right(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

inline std::string &str_trim(std::string &s)
{
    return str_trim_left(str_trim_right(s));
}

int64_t parseLong_ex(const char* input);
void parseIPv4_ex(const char* input, uint8_t* target);
bool secureArgCount(std::vector<std::string> vec, size_t len);

int ExecAndWait(const char* path, const char* argv[]);
std::string ExecAndGetOutput(const char* path, const char* argv[], int& retvalue);

std::string SanitizePath(const char* path);

bool ListDirectory(const char* dirName, std::vector<std::string> &target);

#endif

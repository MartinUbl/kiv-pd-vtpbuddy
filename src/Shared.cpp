#include "general.h"
#include "Shared.h"

#include <exception>
#include <sstream>

int64_t parseLong_ex(const char* input)
{
    int64_t tmp;
    size_t off;

    tmp = std::stoll(input, &off);
    if (off != strlen(input))
        throw std::invalid_argument(input);

    return tmp;
}

void parseIPv4_ex(const char* input, uint8_t* target)
{
    std::istringstream ss(input);
    std::string token;
    int64_t orig;

    size_t i = 0;
    while (std::getline(ss, token, '.'))
    {
        if (i > 3)
            throw std::invalid_argument(input);

        orig = parseLong_ex(token.c_str());
        if (orig < 0 || orig > 255)
            throw std::invalid_argument(input);

        target[i++] = (uint8_t)orig;
    }

    if (i != 4)
        throw std::invalid_argument(input);
}

bool secureArgCount(std::vector<std::string> vec, size_t len)
{
    if (vec.size() < len)
        throw std::domain_error("Invalid argument count " + std::to_string(vec.size()) + ", expected " + std::to_string(len));

    return true;
}

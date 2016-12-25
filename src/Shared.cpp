#include "general.h"
#include "Shared.h"

#include <exception>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <dirent.h>

int64_t parseLong_ex(const char* input)
{
    int64_t tmp;
    size_t off;

    // expected parsed length is input length; but cut endlines, spaces and tabs from the end of input
    size_t expected = strlen(input);
    for (; expected > 0; expected--)
    {
        if (input[expected-1] != '\r' && input[expected-1] != '\n' && input[expected-1] != ' ' && input[expected-1] != '\t')
            break;
    }

    tmp = std::stoll(input, &off);
    if (off != expected)
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

        target[3 - (i++)] = (uint8_t)orig;
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

int ExecAndWait(const char* path, const char* argv[])
{
    int status = -1;
    pid_t chld = fork();

    if (chld == 0)
    {
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        close(STDIN_FILENO);

        // in fact, we don't care if execvp is successfull
        execvp(path, (char* const*)argv);

        exit(-1);
    }
    else if (chld < 0)
        std::cerr << "Could not fork main process" << std::endl;

    if (waitpid(chld, &status, 0) <= -1)
        return -1;

    return status;
}

std::string ExecAndGetOutput(const char* path, const char* argv[], int& retvalue)
{
    retvalue = -1;
    int pipefd[2];
    pipe(pipefd);

    pid_t chld = fork();

    if (chld == 0)
    {
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);

        close(pipefd[0]);
        close(pipefd[1]);
        close(STDIN_FILENO);

        // in fact, we don't care if execvp is successfull
        execvp(path, (char* const*)argv);

        exit(-1);
    }
    else if (chld < 0)
        std::cerr << "Could not fork main process" << std::endl;

    // we won't need write end of pipe
    close(pipefd[1]);

    std::string out;
    char buf[128];

    int res;
    while ((res = read(pipefd[0], buf, 127)) > 0)
    {
        buf[res] = '\0';
        out += buf;
    }

    close(pipefd[0]);

    if (waitpid(chld, &retvalue, 0) <= -1)
        return "ERR";

    return out;
}

std::string SanitizePath(const char* path)
{
    if (path == nullptr || path[0] == '\0')
        return "";

    std::string out = "";

    if (path[0] != '/')
        out += "/";

    out += path;

    // disallow root as destination (well, you never know...)
    if (out == "/")
        return "";

    if (path[strlen(path)-1] != '/')
        out += "/";

    return out;
}

bool ListDirectory(const char* dirName, std::vector<std::string> &target)
{
    DIR* dirdesc;
    struct dirent *dirp;

    dirdesc = opendir(dirName);

    if (!dirdesc)
        return false;

    while ((dirp = readdir(dirdesc)) != nullptr)
        target.push_back(dirp->d_name);

    closedir(dirdesc);
    return true;
}

#include "GitCommandExecutor/GitCommandExecutorUnix.hpp"

#include <stdexcept>
#include <sys/wait.h>
#include <unistd.h>

namespace CppGit {

GitCommandOutput GitCommandExecutorUnix::executeImpl(const std::string_view path, const std::string_view command, const std::vector<std::string_view>& args)
{
    createPipes();
    pid = fork();
    if (pid == -1)
    {
        throw std::runtime_error("Failed to fork");
    }

    if (pid == 0)
    {
        childProcess(path, command, args);
    }
    else
    {
        return parentProcess();
    }

    return GitCommandOutput();
}

void GitCommandExecutorUnix::createPipes()
{
    if (pipe(stdoutPipe.data()) == -1)
    {
        throw std::runtime_error("Failed to create stdout pipe");
    }
    if (pipe(stderrPipe.data()) == -1)
    {
        throw std::runtime_error("Failed to create stderr pipe");
    }
}

GitCommandOutput GitCommandExecutorUnix::parentProcess()
{
    close(stdoutPipe[1]);
    close(stderrPipe[1]);

    int status;
    if (wait(&status) == -1)
    {
        throw std::runtime_error("Failed to waitpid.");
    }

    int returnCode = WEXITSTATUS(status);

    char buffer[256];

    std::string stdoutStr;
    ssize_t bytesReadStdOut;
    while ((bytesReadStdOut = read(stdoutPipe[0], buffer, sizeof(buffer) - 1)) > 0)
    {
        buffer[bytesReadStdOut] = '\0'; // Null-terminate the buffer correctly
        stdoutStr += buffer;
    }
    if (bytesReadStdOut == -1)
    {
        throw std::runtime_error("Failed to read stdout");
    }

    if (stdoutStr.back() == '\n')
    {
        stdoutStr.pop_back();
    }

    std::string stderrStr;
    ssize_t bytesReadStdErr;
    while ((bytesReadStdErr = read(stderrPipe[0], buffer, sizeof(buffer) - 1)) > 0)
    {
        buffer[bytesReadStdErr] = '\0'; // Null-terminate the buffer correctly
        stderrStr += buffer;
    }
    if (bytesReadStdErr == -1)
    {
        throw std::runtime_error("Failed to read stderr");
    }

    if (stderrStr.back() == '\n')
    {
        stderrStr.pop_back();
    }

    return GitCommandOutput{ returnCode, stdoutStr, stderrStr };
}

void GitCommandExecutorUnix::childProcess(const std::string_view path, const std::string_view command, const std::vector<std::string_view>& args)
{
    close(stdoutPipe[0]);
    close(stderrPipe[0]);

    if (dup2(stdoutPipe[1], STDOUT_FILENO) == -1 || dup2(stderrPipe[1], STDERR_FILENO) == -1)
    {
        throw std::runtime_error("Failed to redirect stdout/stderr");
    }

    close(stdoutPipe[1]);
    close(stderrPipe[1]);

    std::vector<char*> argv;

    argv.reserve(args.size() + 4);
    argv.emplace_back(const_cast<char*>("git"));
    argv.emplace_back(const_cast<char*>("-C"));
    argv.emplace_back(const_cast<char*>(path.data()));
    argv.emplace_back(const_cast<char*>(command.data()));

    for (const auto& arg : args)
    {
        if (!arg.empty())
        {
            argv.emplace_back(const_cast<char*>(arg.data()));
        }
    }
    argv.emplace_back(nullptr);

    execvp("git", argv.data());

    perror("execlp");
    exit(EXIT_FAILURE);
}

} // namespace CppGit

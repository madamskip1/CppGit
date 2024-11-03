#include "GitCommandExecutor/GitCommandExecutorUnix.hpp"

#include "GitCommandExecutor/GitCommandExecutor.hpp"

#include <fcntl.h>
#include <stdexcept>
#include <sys/wait.h>
#include <unistd.h>

namespace CppGit {

auto GitCommandExecutorUnix::executeImpl(const std::vector<std::string>& environmentVariables, const std::string_view repoPath, const std::string_view command, const std::vector<std::string>& args) -> GitCommandOutput
{
    createPipes();
    pid = fork();
    if (pid == -1)
    {
        throw std::runtime_error("Failed to fork");
    }

    if (pid == 0)
    {
        childProcess(environmentVariables, repoPath, command, args);
    }
    else
    {
        return parentProcess();
    }

    return GitCommandOutput();
}

auto GitCommandExecutorUnix::createPipes() -> void
{
    if (pipe2(stdoutPipe.data(), O_CLOEXEC) == -1)
    {
        throw std::runtime_error("Failed to create stdout pipe");
    }
    if (pipe2(stderrPipe.data(), O_CLOEXEC) == -1)
    {
        throw std::runtime_error("Failed to create stderr pipe");
    }
}

auto GitCommandExecutorUnix::parentProcess() -> GitCommandOutput
{
    close(stdoutPipe[1]);
    close(stderrPipe[1]);

    int status{};
    if (wait(&status) == -1)
    {
        throw std::runtime_error("Failed to waitpid.");
    }

    int returnCode = WEXITSTATUS(status);

    constexpr int bufferSize = 256;
    std::array<char, bufferSize> buffer{};

    std::string stdoutStr;
    ssize_t bytesReadStdOut{};

    while ((bytesReadStdOut = read(stdoutPipe[0], buffer.data(), bufferSize)) > 0)
    {
        stdoutStr.append(buffer.data(), static_cast<std::size_t>(bytesReadStdOut));
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
    ssize_t bytesReadStdErr{};

    while ((bytesReadStdErr = read(stderrPipe[0], buffer.data(), bufferSize)) > 0)
    {
        stderrStr.append(buffer.data(), static_cast<std::size_t>(bytesReadStdErr));
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

auto GitCommandExecutorUnix::childProcess(const std::vector<std::string>& environmentVariables, const std::string_view repoPath, const std::string_view command, const std::vector<std::string>& args) -> void
{
    close(stdoutPipe[0]);
    close(stderrPipe[0]);

    if (dup2(stdoutPipe[1], STDOUT_FILENO) == -1 || dup2(stderrPipe[1], STDERR_FILENO) == -1)
    {
        throw std::runtime_error("Failed to redirect stdout/stderr");
    }

    close(stdoutPipe[1]);
    close(stderrPipe[1]);

    std::vector<const char*> argv;

    argv.reserve(args.size() + 5);
    argv.emplace_back(GIT_EXECUTABLE);
    argv.emplace_back("-C");
    argv.push_back(repoPath.data());
    argv.push_back(command.data());

    for (const auto& arg : args)
    {
        if (!arg.empty())
        {
            argv.push_back(arg.data());
        }
    }
    argv.emplace_back(nullptr);

    if (environmentVariables.empty())
    {
        execvp(GIT_EXECUTABLE, const_cast<char* const*>(argv.data()));
        perror("execvp");
    }
    else
    {
        std::vector<const char*> envp;

        for (char** env = environ; *env != nullptr; ++env)
        {
            envp.push_back(*env);
        }

        envp.reserve(envp.size() + environmentVariables.size() + 1);

        for (const auto& envVar : environmentVariables)
        {
            if (!envVar.empty())
            {
                envp.push_back(envVar.data());
            }
        }

        envp.push_back(nullptr);

        execvpe(GIT_EXECUTABLE, const_cast<char* const*>(argv.data()), const_cast<char* const*>(envp.data()));
        perror("execvpe");
    }
    exit(EXIT_FAILURE);
}

} // namespace CppGit

#include "GitCommandExecutor/GitCommandExecutorUnix.hpp"
#include <stdexcept>
#include <unistd.h>
#include <sys/wait.h>

namespace CppGit
{
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
        
        fd_set readFDs;
        FD_ZERO(&readFDs);
        FD_SET(stdoutPipe[0], &readFDs);
        FD_SET(stderrPipe[0], &readFDs);
        int maxFD = std::max(stdoutPipe[0], stderrPipe[0]) + 1;
        if (select(maxFD, &readFDs, nullptr, nullptr, nullptr) == -1)
        {
            throw std::runtime_error("Failed to select");
        }

        char buffer[512];

        std::string stdout;
        if (FD_ISSET(stdoutPipe[0], &readFDs))
        {
            while (read(stdoutPipe[0], buffer, sizeof(buffer)) > 0)
            {
                stdout += buffer;
            }
        }

        std::string stderr;;
        if (FD_ISSET(stderrPipe[0], &readFDs))
        {
            while (read(stderrPipe[0], buffer, sizeof(buffer)) > 0)
            {
                stderr += buffer;
            }
        }

        close(stdoutPipe[0]);
        close(stderrPipe[0]);

        int status;
        if (waitpid(pid, &status, 0) == -1)
        {
            throw std::runtime_error("Failed to waitpid");
        }

        int returnCode = WEXITSTATUS(status);

        return GitCommandOutput{returnCode, stdout, stderr};
    }

    void GitCommandExecutorUnix::childProcess(const std::string_view path, const std::string_view command, const std::vector<std::string_view>& args)
    {
        close(stdoutPipe[0]);
        close(stderrPipe[0]);

        if (dup2(stdoutPipe[1], STDOUT_FILENO) == -1 ||
            dup2(stderrPipe[1], STDERR_FILENO) == -1)
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
            argv.emplace_back(const_cast<char*>(arg.data()));
        }
        argv.emplace_back(nullptr);

        execvp("git", argv.data());

        perror("execlp");
        exit(EXIT_FAILURE);
    }
}

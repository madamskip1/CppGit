#pragma once

#include "GitCommandExecutor.hpp"
#include "GitCommandOutput.hpp"
#include <array>

namespace CppGit
{
    class GitCommandExecutorUnix : public GitCommandExecutor
    {
    public:
        GitCommandExecutorUnix() = default;
        ~GitCommandExecutorUnix() override = default;

    private:
        GitCommandOutput executeImpl(const std::string_view path, const std::string_view command, const std::vector<std::string_view>& args) override;

        void createPipes();
        GitCommandOutput parentProcess();
        void childProcess(const std::string_view path, const std::string_view command, const std::vector<std::string_view>& args);
        
        pid_t pid;
        std::array<int, 2> stdoutPipe;
        std::array<int, 2> stderrPipe;
    };
}
#pragma once

#include "GitCommandExecutor.hpp"
#include "GitCommandOutput.hpp"

#include <array>
#include <string_view>
#include <vector>

namespace CppGit {
class GitCommandExecutorUnix : public GitCommandExecutor
{
public:
    static constexpr const char* const GIT_EXECUTABLE = "git";
    GitCommandExecutorUnix() = default;

private:
    auto executeImpl(const std::vector<std::string>& environmentVariables, const std::string_view repoPath, const std::string_view command, const std::vector<std::string>& args) -> GitCommandOutput override;

    auto createPipes() -> void;
    auto parentProcess() -> GitCommandOutput;
    [[noreturn]] auto childProcess(const std::vector<std::string>& environmentVariables, const std::string_view repoPath, const std::string_view command, const std::vector<std::string>& args) -> void;

    pid_t pid{};
    std::array<int, 2> stdoutPipe{};
    std::array<int, 2> stderrPipe{};
};

} // namespace CppGit

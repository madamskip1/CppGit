#pragma once

#include "GitCommandExecutor.hpp"
#include "GitCommandOutput.hpp"

#include <array>

namespace CppGit {
class GitCommandExecutorUnix : public GitCommandExecutor
{
public:
    GitCommandExecutorUnix() = default;
    ~GitCommandExecutorUnix() override = default;

private:
    auto executeImpl(const std::string_view path, const std::string_view command, const std::vector<std::string>& args) -> GitCommandOutput override;

    auto createPipes() -> void;
    auto parentProcess() -> GitCommandOutput;
    [[noreturn]] auto childProcess(const std::string_view path, const std::string_view command, const std::vector<std::string>& args) -> void;

    pid_t pid{};
    std::array<int, 2> stdoutPipe{};
    std::array<int, 2> stderrPipe{};
};

} // namespace CppGit

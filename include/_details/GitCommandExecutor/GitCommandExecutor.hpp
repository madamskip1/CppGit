#pragma once

#include "GitCommandOutput.hpp"

#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace CppGit {


class GitCommandExecutor
{
public:
    virtual ~GitCommandExecutor() = default;

    template <typename... Args>
    auto execute(const std::vector<std::string>& environmentVariables, const std::string_view repoPath, const std::string_view command, Args&&... args)
        -> GitCommandOutput
        requires((sizeof...(Args) != 1) || !std::conjunction_v<std::is_same<std::decay_t<Args>, std::vector<std::string>>...>)
    {
        if constexpr (sizeof...(args) == 0)
        {
            return executeImpl(environmentVariables, repoPath, command, {});
        }
        else
        {
            auto arguments = std::vector<std::string>{};
            arguments.reserve(sizeof...(args));

            (arguments.emplace_back(std::forward<Args>(args)), ...);

            return executeImpl(environmentVariables, repoPath, command, arguments);
        }
    }

    auto execute(const std::vector<std::string>& environmentVariables, const std::string_view repoPath, const std::string_view command, const std::vector<std::string>& args) -> GitCommandOutput;

protected:
    GitCommandExecutor() = default;
    GitCommandExecutor(const GitCommandExecutor&) = default;
    GitCommandExecutor(GitCommandExecutor&&) = default;
    auto operator=(const GitCommandExecutor&) -> GitCommandExecutor& = default;
    auto operator=(GitCommandExecutor&&) -> GitCommandExecutor& = default;

    virtual auto executeImpl(const std::vector<std::string>& environmentVariables, const std::string_view repoPath, const std::string_view command, const std::vector<std::string>& args) -> GitCommandOutput = 0;
};

} // namespace CppGit

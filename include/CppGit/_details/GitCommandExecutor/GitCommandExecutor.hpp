#pragma once

#include "GitCommandOutput.hpp"

#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace CppGit {

/// @brief Base class for executing git commands
class GitCommandExecutor
{
public:
    virtual ~GitCommandExecutor() = default;

    /// @brief Execute a git command
    /// @tparam Args Types of the arguments
    /// @param environmentVariables Environment variables to set before executing the command
    /// @param repoPath Path to the repository
    /// @param command Command to execute
    /// @param args Arguments to pass to the command
    /// @return Output of the command
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

    /// @brief Execute a git command
    /// @param environmentVariables Environment variables to set before executing the command
    /// @param repoPath Path to the repository
    /// @param command Command to execute
    /// @param args Arguments to pass to the command
    /// @return Output of the command
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

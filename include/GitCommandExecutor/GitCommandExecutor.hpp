#pragma once

#include "GitCommandOutput.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace CppGit {

constexpr const char* const GIT_EXECUTABLE = "git";

class GitCommandExecutor
{
public:
    virtual ~GitCommandExecutor() = default;

    template <typename... Args>
    auto execute(const std::string_view path, const std::string_view command, Args&&... args)
        -> std::enable_if_t<(sizeof...(Args) != 1) || !std::conjunction_v<std::is_same<std::decay_t<Args>, std::vector<std::string>>...>, GitCommandOutput>
    {
        if constexpr (sizeof...(args) == 0)
        {
            return executeImpl(path, command, {});
        }
        else
        {
            auto arguments = std::vector<std::string>{};
            arguments.reserve(sizeof...(args));

            (arguments.emplace_back(std::forward<Args>(args)), ...);

            return executeImpl(path, command, arguments);
        }
    }

    auto execute(const std::string_view path, const std::string_view command, const std::vector<std::string>& args) -> GitCommandOutput
    {
        return executeImpl(path, command, args);
    }

protected:
    virtual auto executeImpl(const std::string_view path, const std::string_view command, const std::vector<std::string>& args) -> GitCommandOutput = 0;
};

} // namespace CppGit

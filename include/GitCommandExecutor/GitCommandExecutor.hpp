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
    auto execute(std::string_view path, std::string_view command, Args... args) -> GitCommandOutput
    {
        auto arguments = std::vector<std::string_view>{ args... };
        return executeImpl(path, command, arguments);
    }

    auto execute(std::string_view path, std::string_view command, const std::vector<std::string_view>& args) -> GitCommandOutput
    {
        return executeImpl(path, command, args);
    }

    auto execute(std::string_view path, std::string_view command, const std::vector<std::string>& args) -> GitCommandOutput
    {
        std::vector<std::string_view> argsView;
        argsView.reserve(args.size());
        for (const auto& arg : args)
        {
            argsView.push_back(arg);
        }
        return executeImpl(path, command, argsView);
    }

protected:
    virtual auto executeImpl(const std::string_view path, const std::string_view command, const std::vector<std::string_view>& args) -> GitCommandOutput = 0;
};

} // namespace CppGit

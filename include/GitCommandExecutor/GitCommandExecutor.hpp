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
    GitCommandOutput execute(std::string_view path, std::string_view command, Args... args)
    {
        auto arguments = std::vector<std::string_view>{ args... };
        return executeImpl(path, command, arguments);
    }

    GitCommandOutput execute(std::string_view path, std::string_view command, const std::vector<std::string_view>& args)
    {
        return executeImpl(path, command, args);
    }

    GitCommandOutput execute(std::string_view path, std::string_view command, const std::vector<std::string>& args)
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
    virtual GitCommandOutput executeImpl(const std::string_view path, const std::string_view command, const std::vector<std::string_view>& args) = 0;
};

} // namespace CppGit

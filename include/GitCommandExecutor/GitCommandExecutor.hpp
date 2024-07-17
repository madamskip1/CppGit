#pragma once

#include <string>
#include <string_view>
#include <vector>
#include "GitCommandOutput.hpp"

namespace CppGit 
{
    constexpr const char* const GIT_EXECUTABLE = "git";

    class GitCommandExecutor
    {
    public:
        virtual ~GitCommandExecutor() = default;

        template<typename... Args>
        GitCommandOutput execute(std::string_view path, std::string_view command, Args... args)
        {
            auto arguments = std::vector<std::string_view> {args...};
            return executeImpl(path, command, arguments);
        }

    protected:
        virtual GitCommandOutput executeImpl(const std::string_view path, const std::string_view command, const std::vector<std::string_view>& args) = 0;
    };
} // namespace CppGit
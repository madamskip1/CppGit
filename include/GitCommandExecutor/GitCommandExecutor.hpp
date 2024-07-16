#pragma once

#include <string>
#include <string_view>
#include "GitCommandOutput.hpp"

namespace CppGit 
{
    constexpr const char* const GIT_EXECUTABLE = "git";

    class GitCommandExecutor
    {
    public:
        virtual ~GitCommandExecutor() = default;
        virtual GitCommandOutput execute(std::string_view command, std::string_view path = "") = 0;
    };
} // namespace CppGit
#pragma once

#include <string>
#include <vector>

namespace CppGit
{
    class Parser
    {
    public:
        Parser() = delete;
        virtual ~Parser() = default;

    protected:
        static std::vector<std::string_view> split(const std::string_view line, const char delimiter);
    };
}
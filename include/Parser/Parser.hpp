#pragma once

#include <cstring>
#include <string>
#include <vector>

namespace CppGit {

class Parser
{
public:
    Parser() = delete;
    virtual ~Parser() = default;

    template <typename T>
    static auto split(const std::string_view line, const T& delimiter) -> std::vector<std::string_view>
    {
        const auto delimiterSize = getDelimiterSize(delimiter);

        std::vector<std::string_view> result;
        std::size_t start = 0;
        std::size_t end = line.find(delimiter);

        while (end != std::string::npos)
        {
            result.emplace_back(line.substr(start, end - start));
            start = end + delimiterSize;
            end = line.find(delimiter, start);
        }

        result.emplace_back(line.substr(start, end - start));

        return result;
    }

    template <typename T>
    static auto getDelimiterSize(const T& delimiter) -> std::size_t
    {
        if constexpr (std::is_same_v<T, char>)
        {
            return std::size_t{ 1 };
        }
        else if constexpr (std::is_same_v<T, std::string_view> || std::is_same_v<T, std::string>)
        {
            return delimiter.size();
        }
        else
        {
            return std::strlen(delimiter);
        }
    }
};

} // namespace CppGit

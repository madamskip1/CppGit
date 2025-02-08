#pragma once

#include <cstring>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace CppGit {

class Parser
{
public:
    template <typename T>
    static auto splitToStringViewsVector(const std::string_view line, const T& delimiter) -> std::vector<std::string_view>
    {
        return splitImpl<std::string_view>(line, delimiter);
    }

    template <typename T>
    static auto splitToStringsVector(const std::string_view line, const T& delimiter) -> std::vector<std::string>
    {
        return splitImpl<std::string>(line, delimiter);
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

    static auto string_viewIteratorToString_view(const std::string_view::const_iterator& begin, const std::string_view::const_iterator& end) -> std::string_view
    {
        auto count = static_cast<std::string_view::size_type>(end - begin);
        return std::string_view{ begin, count };
    }

private:
    template <typename ReturnT, typename DelimiterT>
    static auto splitImpl(const std::string_view line, const DelimiterT& delimiter) -> std::vector<ReturnT>
    {
        const auto delimiterSize = getDelimiterSize(delimiter);

        auto result = std::vector<ReturnT>{};
        auto start = std::size_t{ 0 };
        auto end = line.find(delimiter);

        while (end != std::string::npos)
        {
            result.emplace_back(line.substr(start, end - start));
            start = end + delimiterSize;
            end = line.find(delimiter, start);
        }

        result.emplace_back(line.substr(start, end - start));

        return result;
    }
};

} // namespace CppGit

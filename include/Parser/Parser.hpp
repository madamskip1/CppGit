#pragma once

#include <cstring>
#include <string>
#include <vector>

namespace CppGit {

class Parser
{
public:
    virtual ~Parser() = default;

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

        std::vector<ReturnT> result;
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
};

} // namespace CppGit

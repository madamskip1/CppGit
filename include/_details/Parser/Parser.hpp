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
    /// @brief Split content by delimiter
    /// @tparam T Type of delimiter
    /// @param line String view to split
    /// @param delimiter Delimiter
    /// @return Splitted string views vector
    template <typename T>
    static auto splitToStringViewsVector(const std::string_view line, const T& delimiter) -> std::vector<std::string_view>
    {
        return splitImpl<std::string_view>(line, delimiter);
    }

    /// @brief Split content by delimiter
    /// @tparam T Type of delimiter
    /// @param line String view to split
    /// @param delimiter Delimiter
    /// @return Splitted strings vector
    template <typename T>
    static auto splitToStringsVector(const std::string_view line, const T& delimiter) -> std::vector<std::string>
    {
        return splitImpl<std::string>(line, delimiter);
    }

    /// @brief Get size of delimiter
    /// @tparam T Type of delimiter
    /// @param delimiter Delimiter
    /// @return Size of delimiter
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

    /// @brief Convert string_view iterator range to string_view
    /// @param begin Begin string_view iterator
    /// @param end End string_view iterator
    /// @return String_view
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

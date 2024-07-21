#include "Parser/Parser.hpp"

namespace CppGit {

std::vector<std::string_view> Parser::split(const std::string_view line, const char delimiter)
{
    std::vector<std::string_view> result;
    std::size_t start = 0;
    std::size_t end = line.find(delimiter);

    while (end != std::string::npos)
    {
        result.emplace_back(line.substr(start, end - start));
        start = end + 1;
        end = line.find(delimiter, start);
    }

    result.emplace_back(line.substr(start, end));

    return result;
}

} // namespace CppGit

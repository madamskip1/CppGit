#include "Parser/IndexParser.hpp"

#include <regex>

namespace CppGit {

IndexEntry IndexParser::parseStageDetailedEntry(const std::string_view indexEntryLine)
{
    constexpr auto pattern = R"((^\d{6})\s+(.{40})\s+(\d)\s+(\S+)$)";
    auto match = std::cmatch{};
    auto regex = std::regex{ pattern };

    if (!std::regex_match(indexEntryLine.begin(), indexEntryLine.end(), match, regex))
    {
        throw std::runtime_error("Invalid index entry line");
    }
    return IndexEntry{ match[1].str(), match[2].str(), std::stoi(match[3].str()), match[4].str() };
}

std::vector<IndexEntry> IndexParser::parseStageDetailedList(const std::string_view indexContent)
{
    auto splitIndexContent = split(indexContent, '\n');
    std::vector<IndexEntry> indexEntries;
    indexEntries.reserve(splitIndexContent.size());

    for (const auto& indexEntryLine : splitIndexContent)
    {
        indexEntries.push_back(parseStageDetailedEntry(indexEntryLine));
    }

    return indexEntries;
}


std::vector<std::string> IndexParser::parseStageSimpleCacheList(const std::string_view indexContent)
{
    auto listSV = split(indexContent, '\n');
    return std::vector<std::string>{ listSV.begin(), listSV.end() };
}

} // namespace CppGit

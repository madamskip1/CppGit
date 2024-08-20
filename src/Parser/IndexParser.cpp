#include "Parser/IndexParser.hpp"

#include <regex>

namespace CppGit {

auto IndexParser::parseStageDetailedEntry(const std::string_view indexEntryLine) -> IndexEntry
{
    constexpr auto pattern = R"((^\d{6})\s+(.{40})\s+(\d)\s+(\S+)$)";
    auto match = std::cmatch{};

    if (auto regex = std::regex{ pattern }; !std::regex_match(indexEntryLine.begin(), indexEntryLine.end(), match, regex))
    {
        throw std::runtime_error("Invalid index entry line");
    }
    return IndexEntry{ match[1].str(), match[2].str(), std::stoi(match[3].str()), match[4].str() };
}

auto IndexParser::parseStageDetailedList(const std::string_view indexContent) -> std::vector<IndexEntry>
{
    auto splitIndexContent = split(indexContent, '\n');
    std::vector<IndexEntry> indexEntries;
    indexEntries.reserve(splitIndexContent.size());

    for (const auto& indexEntryLine : splitIndexContent)
    {
        if (indexEntryLine.empty())
        {
            continue;
        }
        indexEntries.push_back(parseStageDetailedEntry(indexEntryLine));
    }

    return indexEntries;
}


auto IndexParser::parseStageSimpleCacheList(const std::string_view indexContent) -> std::vector<std::string>
{
    auto listSV = split(indexContent, '\n');
    auto result = std::vector<std::string>{};
    result.reserve(listSV.size());
    for (const auto& entry : listSV)
    {
        if (entry.empty())
        {
            continue;
        }
        result.emplace_back(entry);
    }
    result.shrink_to_fit();
    return result;
}

} // namespace CppGit

#include "CppGit/_details/Parser/IndexParser.hpp"

#include "CppGit/Index.hpp"

#include <algorithm>
#include <charconv>
#include <iterator>
#include <regex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace CppGit {

auto IndexParser::parseStageDetailedEntry(const std::string_view indexEntryLine) -> IndexEntry
{
    constexpr auto pattern = R"((^\d{6})\s+(.{40})\s+(\d)\s+(\S+)$)";
    auto match = std::cmatch{};

    if (const auto regex = std::regex{ pattern }; !std::regex_match(indexEntryLine.cbegin(), indexEntryLine.cend(), match, regex))
    {
        throw std::runtime_error("Invalid index entry line");
    }

    auto fileMode = 0;
    std::from_chars(match[1].first, match[1].second, fileMode);
    return IndexEntry{ .fileMode = fileMode, .stageNumber = std::stoi(match[3].str()), .objectHash = match[2].str(), .path = match[4].str() };
}

auto IndexParser::parseStageDetailedList(const std::string_view indexContent) -> std::vector<IndexEntry>
{
    const auto splitIndexContent = splitToStringViewsVector(indexContent, '\n');
    std::vector<IndexEntry> indexEntries;
    indexEntries.reserve(splitIndexContent.size());

    for (const auto& indexEntryLine : splitIndexContent)
    {
        if (indexEntryLine.empty())
        {
            continue;
        }
        indexEntries.emplace_back(parseStageDetailedEntry(indexEntryLine));
    }

    return indexEntries;
}


auto IndexParser::parseStageSimpleCacheList(const std::string_view indexContent) -> std::vector<std::string>
{
    const auto listSV = splitToStringViewsVector(indexContent, '\n');
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


auto IndexParser::getDiffIndexStatus(const std::string_view status) -> DiffIndexStatus
{
    if (status == "A")
    {
        return DiffIndexStatus::ADDED;
    }
    if (status == "D")
    {
        return DiffIndexStatus::DELETED;
    }
    if (status == "M")
    {
        return DiffIndexStatus::MODIFIED;
    }
    if (status == "R")
    {
        return DiffIndexStatus::RENAMED;
    }
    if (status == "C")
    {
        return DiffIndexStatus::COPIED;
    }
    if (status == "T")
    {
        return DiffIndexStatus::TYPE_CHANGED;
    }
    if (status == "U")
    {
        return DiffIndexStatus::UNMERGED;
    }
    if (status == "X")
    {
        return DiffIndexStatus::UNKNOWN;
    }

    return DiffIndexStatus();
}

auto IndexParser::parseDiffIndexWithStatusEntry(const std::string_view diffIndexLine) -> DiffIndexEntry
{
    constexpr auto pattern = R"((\w)\s+(.+))";
    auto match = std::cmatch{};

    if (const auto regex = std::regex{ pattern }; !std::regex_match(diffIndexLine.cbegin(), diffIndexLine.cend(), match, regex))
    {
        throw std::runtime_error("Invalid diff index entry line");
    }

    return DiffIndexEntry{ .path = match[2].str(), .status = getDiffIndexStatus(match[1].str()) };
}

auto IndexParser::parseDiffIndexWithStatus(const std::string_view diffIndexContent) -> std::vector<DiffIndexEntry>
{
    if (diffIndexContent.empty())
    {
        return std::vector<DiffIndexEntry>{};
    }

    const auto splitDiffIndexContent = splitToStringViewsVector(diffIndexContent, '\n');
    auto result = std::vector<DiffIndexEntry>{};
    result.reserve(splitDiffIndexContent.size());

    std::ranges::transform(splitDiffIndexContent, std::back_inserter(result), [](const auto diffIndexLine) {
        return parseDiffIndexWithStatusEntry(diffIndexLine);
    });

    return result;
}

auto IndexParser::getLsFilesStatus(const std::string_view status) -> LsFilesStatus
{
    if (status == "H")
    {
        return LsFilesStatus::TRACKED_NOT_UNMERGED_NOT_SKIP_WORKTREE;
    }
    if (status == "S")
    {
        return LsFilesStatus::TRACKED_SKIP_WORKTREE;
    }
    if (status == "M")
    {
        return LsFilesStatus::TRACKED_UNMERGED;
    }
    if (status == "R")
    {
        return LsFilesStatus::TRACKED_DELETED;
    }
    if (status == "C")
    {
        return LsFilesStatus::TRACKED_MODIFIED;
    }
    if (status == "K")
    {
        return LsFilesStatus::UNTRACKED_CONFLICTING;
    }
    if (status == "?")
    {
        return LsFilesStatus::UNTRACKED;
    }
    if (status == "U")
    {
        return LsFilesStatus::RESOLVE_UNDO;
    }

    return LsFilesStatus();
}

auto IndexParser::parseLsFilesEntry(const std::string_view lsFilesLine) -> LsFilesEntry
{
    constexpr auto pattern = R"(([\w\?])\s+(.+))";
    auto match = std::cmatch{};

    if (const auto regex = std::regex{ pattern }; !std::regex_match(lsFilesLine.cbegin(), lsFilesLine.cend(), match, regex))
    {
        throw std::runtime_error("Invalid ls-files entry line");
    }

    return LsFilesEntry{ .path = match[2].str(), .status = getLsFilesStatus(match[1].str()) };
}

auto IndexParser::parseLsFilesList(const std::string_view lsFilesContent) -> std::vector<LsFilesEntry>
{
    if (lsFilesContent.empty())
    {
        return std::vector<LsFilesEntry>{};
    }

    const auto splitLsFilesContent = splitToStringViewsVector(lsFilesContent, '\n');
    auto result = std::vector<LsFilesEntry>{};
    result.reserve(splitLsFilesContent.size());

    std::ranges::transform(splitLsFilesContent, std::back_inserter(result), [](const auto& lsFilesLine) {
        return parseLsFilesEntry(lsFilesLine);
    });

    return result;
}

} // namespace CppGit

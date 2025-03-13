#include "CppGit/_details/Parser/DiffParser.hpp"

#include "CppGit/DiffFile.hpp"

#include <algorithm>
#include <charconv>
#include <cstddef>
#include <iterator>
#include <regex>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace CppGit {

auto DiffParser::parse(const std::string_view diffContent) -> std::vector<DiffFile>
{
    const auto splittedDiff = splitToStringViewsVector(diffContent, '\n');
    std::vector<HeaderLine> headersLines;

    std::vector<DiffFile> diffFiles;
    auto diffFile = DiffFile{};
    diffFile.diffStatus = DiffStatus::UNKNOWN;

    const auto endIterator = splittedDiff.cend();
    for (auto iterator = splittedDiff.cbegin(); iterator < endIterator; ++iterator)
    {
        const auto line = *iterator;
        switch (currentState)
        {
        case ParseState::WAITING_FOR_DIFF: {
            currentState = ParseState::HEADER;
            headersLines.clear();

            const auto diffLine = parseDiffLine(line);
            diffFile.isCombined = diffLine.isCombined;
            diffFile.fileA = diffLine.fileA;
            diffFile.fileB = diffLine.fileB;
            break;
        }

        case ParseState::HEADER: {
            const auto lastHeaderLineType = headersLines.empty() ? HeaderLineType::NO_LINE : headersLines.back().type;
            const auto headerLine = parseHeaderLine(line, lastHeaderLineType);


            headersLines.push_back(headerLine);
            processHeaderLine(headerLine, diffFile);

            if (const auto nextLine = peakNextLine(iterator, endIterator); nextLine == "Binary files differ")
            {
                currentState = ParseState::BINARY_FILE;
            }
            else if (nextLine.starts_with("---"))
            {
                currentState = ParseState::HUNK_FILE_A;
            }


            break;
        }

        case ParseState::HUNK_FILE_A: {
            diffFile.fileA = parseHunkFileLine(line, "a/");
            currentState = ParseState::HUNK_FILE_B;
            break;
        }
        case ParseState::HUNK_FILE_B:
            diffFile.fileB = parseHunkFileLine(line, "b/");
            currentState = ParseState::HUNK_HEADER;
            break;

        case ParseState::HUNK_HEADER: {
            auto [rangesBefore, rangeAfter] = parseHunkHeader(line);
            diffFile.hunkRangesBefore = std::move(rangesBefore);
            diffFile.hunkRangeAfter = std::move(rangeAfter);

            currentState = ParseState::HUNK_CONTENT;
            break;
        }

        case ParseState::HUNK_CONTENT:
            diffFile.hunkContent.emplace_back(line);
            break;

        case ParseState::BINARY_FILE:
            diffFile.diffStatus = DiffStatus::BINARY_CHANGED;
            currentState = ParseState::WAITING_FOR_DIFF;
            break;
        }

        if (peakNextLine(iterator, endIterator).starts_with("diff"))
        {
            diffFiles.push_back(std::move(diffFile));
            diffFile = DiffFile{};
            diffFile.diffStatus = DiffStatus::UNKNOWN;
            currentState = ParseState::WAITING_FOR_DIFF;
        }
    }
    if (diffFile.diffStatus != DiffStatus::UNKNOWN)
    {
        diffFiles.push_back(std::move(diffFile));
    }

    return diffFiles;
}


auto DiffParser::parseHeaderLine(const std::string_view line, const HeaderLineType headerLineBefore) -> HeaderLine
{
    static constexpr auto oldModePrefix = "old mode";
    static constexpr auto oldModePattern = R"(^old mode (\d{6})$)";
    static constexpr auto newModePattern = R"(^new mode (\d{6})$)";
    static constexpr auto deletedFilePrefix = "deleted file mode";
    static constexpr auto deletedFilePattern = R"(^deleted file mode (\d{6})$)";
    static constexpr auto newFilePrefix = "new file mode";
    static constexpr auto newFilePattern = R"(^new file mode (\d{6})$)";
    static constexpr auto copyFromPrefix = "copy from";
    static constexpr auto copyFromPattern = R"(^copy from (\S+)$)";
    static constexpr auto copyToPattern = R"(^copy to (\S+)$)";
    static constexpr auto renamedFromprefix = "rename from";
    static constexpr auto renamedFromPattern = R"(^rename from (\S+)$)";
    static constexpr auto renamedToPattern = R"(^rename to (\S+)$)";
    static constexpr auto similarityIndexPrefix = "similarity index";
    static constexpr auto similarityIndexPattern = R"(^similarity index (\d+)%$)";
    static constexpr auto indexPrefix = "index";
    static constexpr auto indexPattern = R"(^index ([\w\d,]+)..([\w\d]+)\s?([\w\d]+)?$)";

    auto getIntValueFromStringViewMatch = [](const std::match_results<std::string_view::const_iterator>& match, std::size_t index) {
        auto value = 0;

        if (match[index].first != match[index].second)
        {
            std::from_chars(match[index].first, match[index].second, value);
        }

        return value;
    };

    auto match = std::match_results<std::string_view::const_iterator>();

    switch (headerLineBefore)
    {
    case HeaderLineType::NO_LINE:
        if (line.starts_with(indexPrefix))
        {
            std::regex_match(line.cbegin(), line.cend(), match, std::regex{ indexPattern });
            return HeaderLine{ .type = HeaderLineType::INDEX, .value = std::make_tuple(string_viewIteratorToString_view(match[1].first, match[1].second), string_viewIteratorToString_view(match[2].first, match[2].second), getIntValueFromStringViewMatch(match, 3)) };
        }
        else if (line.starts_with(newFilePrefix))
        {
            std::regex_match(line.cbegin(), line.cend(), match, std::regex{ newFilePattern });
            return HeaderLine{ .type = HeaderLineType::NEW_FILE, .value = getIntValueFromStringViewMatch(match, 1) };
        }
        else if (line.starts_with(deletedFilePrefix))
        {
            std::regex_match(line.begin(), line.end(), match, std::regex{ deletedFilePattern });
            return HeaderLine{ .type = HeaderLineType::DELETED_FILE, .value = getIntValueFromStringViewMatch(match, 1) };
        }
        else if (line.starts_with(similarityIndexPrefix))
        {
            std::regex_match(line.begin(), line.end(), match, std::regex{ similarityIndexPattern });
            return HeaderLine{ .type = HeaderLineType::SIMILARITY_INDEX, .value = getIntValueFromStringViewMatch(match, 1) };
        }
        else if (line.starts_with(oldModePrefix))
        {
            std::regex_match(line.cbegin(), line.cend(), match, std::regex{ oldModePattern });
            return HeaderLine{ .type = HeaderLineType::OLD_MODE, .value = getIntValueFromStringViewMatch(match, 1) };
        }

        break;

    case HeaderLineType::SIMILARITY_INDEX:
        if (line.starts_with(renamedFromprefix))
        {
            std::regex_match(line.cbegin(), line.cend(), match, std::regex{ renamedFromPattern });
            return HeaderLine{ .type = HeaderLineType::RENAME_FROM, .value = string_viewIteratorToString_view(match[1].first, match[1].second) };
        }
        else if (line.starts_with(copyFromPrefix))
        {
            std::regex_match(line.cbegin(), line.cend(), match, std::regex{ copyFromPattern });
            return HeaderLine{ .type = HeaderLineType::COPY_FROM, .value = string_viewIteratorToString_view(match[1].first, match[1].second) };
        }
        break;

    case HeaderLineType::RENAME_FROM:
        std::regex_match(line.cbegin(), line.cend(), match, std::regex{ renamedToPattern });
        return HeaderLine{ .type = HeaderLineType::RENAME_TO, .value = string_viewIteratorToString_view(match[1].first, match[1].second) };

    case HeaderLineType::COPY_FROM:
        std::regex_match(line.cbegin(), line.cend(), match, std::regex{ copyToPattern });
        return HeaderLine{ .type = HeaderLineType::COPY_TO, .value = string_viewIteratorToString_view(match[1].first, match[1].second) };

    case HeaderLineType::OLD_MODE:
        std::regex_match(line.cbegin(), line.cend(), match, std::regex{ newModePattern });
        return HeaderLine{ .type = HeaderLineType::NEW_MODE, .value = getIntValueFromStringViewMatch(match, 1) };

    case HeaderLineType::RENAME_TO:
    case HeaderLineType::COPY_TO:
    case HeaderLineType::NEW_MODE:
    case HeaderLineType::NEW_FILE:
    case HeaderLineType::DELETED_FILE:
        std::regex_match(line.cbegin(), line.cend(), match, std::regex{ indexPattern });
        return HeaderLine{ .type = HeaderLineType::INDEX, .value = std::make_tuple(string_viewIteratorToString_view(match[1].first, match[1].second), string_viewIteratorToString_view(match[2].first, match[2].second), getIntValueFromStringViewMatch(match, 3)) };
    default:
        break;
    }

    return {};
}


auto DiffParser::parseHunkFileLine(const std::string_view line, const std::string_view prefix) -> std::string
{
    auto file = splitToStringViewsVector(line, ' ')[1];
    removePrefixFromFileIfStartsWith(file, prefix);

    return std::string{ file };
}

auto DiffParser::peakNextLine(std::vector<std::string_view>::const_iterator iterator, const std::vector<std::string_view>::const_iterator endIterator) -> std::string_view
{
    ++iterator;
    return (iterator == endIterator) ? "" : *iterator;
}

auto DiffParser::removePrefixFromFileIfStartsWith(std::string_view& file, const std::string_view prefix) -> void
{
    if (file.starts_with(prefix))
    {
        file.remove_prefix(prefix.size());
    }
}

auto DiffParser::processHeaderLine(const HeaderLine& headerLine, DiffFile& diffFile) -> void
{
    if (headerLine.type == HeaderLineType::INDEX)
    {
        const auto& [indicesBeforeSV, indexAfter, mode] = std::get<std::tuple<std::string_view, std::string_view, int>>(headerLine.value);

        diffFile.indicesBefore = splitToStringsVector(indicesBeforeSV, ',');
        diffFile.indexAfter = std::string{ indexAfter };

        if (mode != 0 && diffFile.newMode == 0)
        {
            diffFile.newMode = mode;
        }

        if (diffFile.diffStatus == DiffStatus::UNKNOWN)
        {
            diffFile.diffStatus = DiffStatus::MODDIFIED;
        }
        else if (diffFile.diffStatus == DiffStatus::RENAMED)
        {
            diffFile.diffStatus = DiffStatus::RENAMED_AND_MODIFIED;
        }
        else if (diffFile.diffStatus == DiffStatus::COPIED)
        {
            diffFile.diffStatus = DiffStatus::COPIED_AND_MODIFIED;
        }
        else if (diffFile.diffStatus == DiffStatus::TYPE_CHANGED)
        {
            diffFile.diffStatus = DiffStatus::TYPE_CHANGED_SYMLINK;
        }

        return;
    }

    if (diffFile.diffStatus == DiffStatus::UNKNOWN)
    {
        if (headerLine.type == HeaderLineType::NEW_FILE)
        {
            diffFile.diffStatus = DiffStatus::NEW;
            diffFile.fileA = "/dev/null";
            diffFile.newMode = std::get<int>(headerLine.value);
        }
        else if (headerLine.type == HeaderLineType::DELETED_FILE)
        {
            diffFile.diffStatus = DiffStatus::DELETED;
            diffFile.fileB = "/dev/null";
            diffFile.oldMode = std::get<int>(headerLine.value);
        }
        else if (headerLine.type == HeaderLineType::RENAME_FROM)
        {
            diffFile.diffStatus = DiffStatus::RENAMED;
            diffFile.fileA = std::get<std::string_view>(headerLine.value);
        }
        else if (headerLine.type == HeaderLineType::OLD_MODE)
        {
            diffFile.diffStatus = DiffStatus::TYPE_CHANGED;
            diffFile.oldMode = std::get<int>(headerLine.value);
        }
        else if (headerLine.type == HeaderLineType::COPY_FROM)
        {
            diffFile.diffStatus = DiffStatus::COPIED;
            diffFile.fileA = std::get<std::string_view>(headerLine.value);
        }
        else if (headerLine.type == HeaderLineType::INDEX)
        {
            diffFile.diffStatus = DiffStatus::MODDIFIED;
        }
        else if (headerLine.type == HeaderLineType::SIMILARITY_INDEX)
        {
            diffFile.similarityIndex = std::get<int>(headerLine.value);
        }
    }
    else if (diffFile.diffStatus == DiffStatus::TYPE_CHANGED)
    {
        diffFile.newMode = std::get<int>(headerLine.value);
    }
    else if (diffFile.diffStatus == DiffStatus::RENAMED || diffFile.diffStatus == DiffStatus::COPIED)
    {
        diffFile.fileB = std::get<std::string_view>(headerLine.value);
    }
}

auto DiffParser::parseHunkHeader(const std::string_view line) -> std::pair<std::vector<std::pair<int, int>>, std::pair<int, int>>
{
    static constexpr auto hunkHeaderPattern = R"(^@{2,} ((?:-\d+(?:,\d+)?\s)+)(\+\d+(?:,\d+)?) @{2,}$)";

    auto match = std::match_results<std::string_view::const_iterator>();
    std::regex_match(line.cbegin(), line.cend(), match, std::regex{ hunkHeaderPattern });
    auto beforeIndicesSV = string_viewIteratorToString_view(match[1].first, match[1].second);
    beforeIndicesSV.remove_suffix(1); // we have extra space at the end. May change regex to avoid this later

    auto beforeIndices = splitToStringViewsVector(beforeIndicesSV, ' ');

    auto hunkRangesBefore = std::vector<std::pair<int, int>>{};

    std::ranges::transform(beforeIndices, std::back_inserter(hunkRangesBefore), [](const auto& range) {
        return parseHunkHeaderRange(range);
    });

    auto hunkRangeAfter = parseHunkHeaderRange(string_viewIteratorToString_view(match[2].first, match[2].second));

    return std::make_pair(std::move(hunkRangesBefore), std::move(hunkRangeAfter));
}


auto DiffParser::parseHunkHeaderRange(const std::string_view range) -> std::pair<int, int>
{
    auto splittedRange = splitToStringViewsVector(range, ',');
    splittedRange[0].remove_prefix(1); // remove the leading '+' or '-', do we need it later?

    auto left = 1;
    std::from_chars(splittedRange[0].cbegin(), splittedRange[0].cend(), left);

    auto right = -1;
    if (splittedRange.size() == 2)
    {
        std::from_chars(splittedRange[1].cbegin(), splittedRange[1].cend(), right);
    }

    return std::make_pair(left, right);
}

auto DiffParser::parseDiffLine(const std::string_view line) -> DiffLine
{
    static constexpr auto pattern = R"(^diff --(\w{2,3}) (\S+)\s?(\S+)?$)";

    auto match = std::match_results<std::string_view::const_iterator>();
    std::regex_match(line.cbegin(), line.cend(), match, std::regex{ pattern });

    auto isCombined = false;
    if (string_viewIteratorToString_view(match[1].first, match[1].second) == "cc")
    {
        isCombined = true;
    }

    auto fileA = string_viewIteratorToString_view(match[2].first, match[2].second);
    removePrefixFromFileIfStartsWith(fileA, "a/");

    auto fileB = std::string_view{};
    if (match[3].matched)
    {
        fileB = string_viewIteratorToString_view(match[3].first, match[3].second);
        removePrefixFromFileIfStartsWith(fileB, "b/");
    }

    return DiffLine{ .isCombined = isCombined, .fileA = fileA, .fileB = fileB };
}


} // namespace CppGit

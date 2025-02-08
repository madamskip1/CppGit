#include "_details/Parser/DiffParser.hpp"

#include "DiffFile.hpp"

#include <charconv>
#include <cstddef>
#include <regex>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace CppGit {

auto DiffParser::parse(const std::string_view diffContent) -> std::vector<DiffFile>
{
    auto splittedDiff = splitToStringViewsVector(diffContent, '\n');
    std::vector<HeaderLine> headersLines;

    std::vector<DiffFile> diffFiles;
    DiffFile diffFile;

    for (auto iterator = splittedDiff.cbegin(); iterator < splittedDiff.cend(); ++iterator)
    {
        auto line = *iterator;
        switch (currentState)
        {
        case ParseState::WAITING_FOR_DIFF: {
            if (line.empty())
            {
                continue;
            }

            if (!line.starts_with("diff"))
            {
                continue;
            }
            currentState = ParseState::HEADER;
            diffFile = DiffFile{};
            headersLines.clear();

            auto diffLine = parseDiffLine(line);
            diffFile.isCombined = diffLine.isCombined ? DiffType::COMBINED : DiffType::NORMAL;
            diffFile.fileA = diffLine.fileA;
            diffFile.fileB = diffLine.fileB;
            break;
        }
        case ParseState::HEADER: {
            auto lastHeaderLineType = headersLines.empty() ? HeaderLineType::NO_LINE : headersLines.back().type;

            if (auto headerLine = parseHeaderLine(line, lastHeaderLineType); headerLine.type == HeaderLineType::END_HEADER)
            {
                if (line.starts_with("diff"))
                {
                    currentState = ParseState::WAITING_FOR_DIFF;
                    --iterator;
                }
                else if (line == "Binary files differ")
                {
                    diffFile.diffStatus = DiffStatus::BINARY_CHANGED;
                    currentState = ParseState::WAITING_FOR_DIFF;
                }
                else
                {
                    currentState = ParseState::HUNK_FILE_A;
                    --iterator;
                }
            }
            else
            {
                if (headerLine.type == HeaderLineType::INDEX)
                {
                    const auto& [indicesBeforeSV, indexAfter, mode] = std::get<std::tuple<std::string_view, std::string_view, int>>(headerLine.value);
                    auto indicesBefore = splitToStringViewsVector(indicesBeforeSV, ',');
                    for (auto indexBefore : indicesBefore)
                    {
                        diffFile.indicesBefore.emplace_back(indexBefore);
                    }
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
                }
                else if (diffFile.diffStatus == DiffStatus::UNKNOWN)
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

                headersLines.push_back(parseHeaderLine(line, lastHeaderLineType));
            }
            break;
        }
        case ParseState::HUNK_FILE_A: {
            auto fileA = splitToStringViewsVector(line, ' ')[1];
            if (fileA.starts_with("a/"))
            {
                fileA.remove_prefix(2);
            }
            diffFile.fileA = fileA;
            currentState = ParseState::HUNK_FILE_B;
            break;
        }
        case ParseState::HUNK_FILE_B: {
            auto fileB = splitToStringViewsVector(line, ' ')[1];
            if (fileB.starts_with("b/"))
            {
                fileB.remove_prefix(2);
            }
            diffFile.fileB = fileB;
            currentState = ParseState::HUNK_HEADER;
            break;
        }
        case ParseState::HUNK_HEADER: {
            auto [rangesBefore, rangeAfter] = parseHunkHeader(line);
            diffFile.hunkRangesBefore = rangesBefore;
            diffFile.hunkRangeAfter = rangeAfter;

            currentState = ParseState::HUNK_CONTENT;

            break;
        }
        case ParseState::HUNK_CONTENT:
            if (line.starts_with("diff"))
            {
                diffFiles.push_back(std::move(diffFile));
                currentState = ParseState::WAITING_FOR_DIFF;
                --iterator;
            }
            else
            {
                diffFile.hunkContent.emplace_back(line);
            }
            break;
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
    static constexpr auto oldModePattern = R"(^old mode (\d{6})$)";
    static constexpr auto newModePattern = R"(^new mode (\d{6})$)";
    static constexpr auto deletedFilePattern = R"(^deleted file mode (\d{6})$)";
    static constexpr auto newFilePattern = R"(^new file mode (\d{6})$)";
    static constexpr auto copyFromPattern = R"(^copy from (\S+)$)";
    static constexpr auto copyToPattern = R"(^copy to (\S+)$)";
    static constexpr auto renamedFromPattern = R"(^rename from (\S+)$)";
    static constexpr auto renamedToPattern = R"(^rename to (\S+)$)";
    static constexpr auto similarityIndexPattern = R"(^similarity index (\d+)%$)";
    static constexpr auto indexPattern = R"(^index ([\w\d,]+)..([\w\d]+)\s?([\w\d]+)?$)";

    auto match = std::match_results<std::string_view::const_iterator>();

    switch (headerLineBefore)
    {
    case HeaderLineType::NO_LINE:
        if (std::regex_match(line.cbegin(), line.cend(), match, std::regex{ indexPattern }))
        {
            return HeaderLine{ .type = HeaderLineType::INDEX, .value = std::make_tuple(string_viewIteratorToString_view(match[1].first, match[1].second), string_viewIteratorToString_view(match[2].first, match[2].second), getIntFromStringViewMatch(match, 3)) };
        }
        else if (std::regex_match(line.cbegin(), line.cend(), match, std::regex{ newFilePattern }))
        {
            return HeaderLine{ .type = HeaderLineType::NEW_FILE, .value = getIntFromStringViewMatch(match, 1) };
        }
        else if (std::regex_match(line.begin(), line.end(), match, std::regex{ deletedFilePattern }))
        {
            return HeaderLine{ .type = HeaderLineType::DELETED_FILE, .value = getIntFromStringViewMatch(match, 1) };
        }
        else if (std::regex_match(line.begin(), line.end(), match, std::regex{ similarityIndexPattern }))
        {
            return HeaderLine{ .type = HeaderLineType::SIMILARITY_INDEX, .value = getIntFromStringViewMatch(match, 1) };
        }
        else if (std::regex_match(line.cbegin(), line.cend(), match, std::regex{ oldModePattern }))
        {
            return HeaderLine{ .type = HeaderLineType::OLD_MODE, .value = getIntFromStringViewMatch(match, 1) };
        }
        break;

    case HeaderLineType::SIMILARITY_INDEX:
        if (std::regex_match(line.cbegin(), line.cend(), match, std::regex{ renamedFromPattern }))
        {
            return HeaderLine{ .type = HeaderLineType::RENAME_FROM, .value = string_viewIteratorToString_view(match[1].first, match[1].second) };
        }
        else if (std::regex_match(line.cbegin(), line.cend(), match, std::regex{ copyFromPattern }))
        {
            return HeaderLine{ .type = HeaderLineType::COPY_FROM, .value = string_viewIteratorToString_view(match[1].first, match[1].second) };
        }
        break;

    case HeaderLineType::RENAME_FROM:
        std::regex_match(line.cbegin(), line.cend(), match, std::regex{ renamedToPattern });

        return HeaderLine{ .type = HeaderLineType::RENAME_TO, .value = string_viewIteratorToString_view(match[1].first, match[1].second) };

    case HeaderLineType::COPY_FROM:
        std::regex_match(line.cbegin(), line.cend(), match, std::regex{ copyToPattern });

        return HeaderLine{ .type = HeaderLineType::COPY_TO, .value = string_viewIteratorToString_view(match[1].first, match[1].second) };

    case HeaderLineType::OLD_MODE: {
        std::regex_match(line.cbegin(), line.cend(), match, std::regex{ newModePattern });

        return HeaderLine{ .type = HeaderLineType::NEW_MODE, .value = getIntFromStringViewMatch(match, 1) };
    }
    case HeaderLineType::RENAME_TO:
    case HeaderLineType::COPY_TO:
    case HeaderLineType::NEW_MODE:
        if (std::regex_match(line.cbegin(), line.cend(), match, std::regex{ indexPattern }))
        {
            return HeaderLine{ .type = HeaderLineType::INDEX, .value = std::make_tuple(string_viewIteratorToString_view(match[1].first, match[1].second), string_viewIteratorToString_view(match[2].first, match[2].second), getIntFromStringViewMatch(match, 3)) };
        }
        return HeaderLine{ .type = HeaderLineType::END_HEADER, .value = 0 };

    case HeaderLineType::NEW_FILE:
    case HeaderLineType::DELETED_FILE: {
        std::regex_match(line.cbegin(), line.cend(), match, std::regex{ indexPattern });

        return HeaderLine{ .type = HeaderLineType::INDEX, .value = std::make_tuple(string_viewIteratorToString_view(match[1].first, match[1].second), string_viewIteratorToString_view(match[2].first, match[2].second), getIntFromStringViewMatch(match, 3)) };
    }
    case HeaderLineType::INDEX:
        return HeaderLine{ .type = HeaderLineType::END_HEADER, .value = 0 };

    case HeaderLineType::END_HEADER:
        break;
    }

    return {};
}

auto DiffParser::getIntFromStringViewMatch(const std::match_results<std::string_view::const_iterator>& match, std::size_t index) -> int
{
    int mode{ 0 };
    if (match[index].first != match[index].second)
    {
        std::from_chars(match[index].first, match[index].second, mode);
    }

    return mode;
}

auto DiffParser::parseHunkHeader(const std::string_view line) -> std::pair<std::vector<std::pair<int, int>>, std::pair<int, int>>
{
    static constexpr auto hunkHeaderPattern = R"(^@{2,} ((?:-\d+(?:,\d+)?\s)+)(\+\d+(?:,\d+)?) @{2,}$)";

    auto match = std::match_results<std::string_view::const_iterator>();
    std::regex_match(line.cbegin(), line.cend(), match, std::regex{ hunkHeaderPattern });

    auto beforeIndicesSV = string_viewIteratorToString_view(match[1].first, match[1].second - 1);
    auto beforeIndices = splitToStringViewsVector(beforeIndicesSV, ' ');

    auto hunkRangesBefore = std::vector<std::pair<int, int>>{};

    for (auto beforeRange : beforeIndices)
    {
        auto hunkRangeBefore = parseHunkHeaderRange(beforeRange);

        hunkRangesBefore.push_back(hunkRangeBefore);
    }

    auto hunkRangeAfter = parseHunkHeaderRange(string_viewIteratorToString_view(match[2].first, match[2].second));

    return std::make_pair(hunkRangesBefore, hunkRangeAfter);
}


auto DiffParser::parseHunkHeaderRange(const std::string_view range) -> std::pair<int, int>
{
    auto splittedRange = splitToStringViewsVector(range, ',');

    int left{ 0 };
    std::from_chars(splittedRange[0].data() + 1, splittedRange[0].data() + splittedRange[0].size(), left);

    int right{ -1 };
    if (splittedRange.size() == 2)
    {
        std::from_chars(splittedRange[1].data(), splittedRange[1].data() + splittedRange[1].size(), right);
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
    if (fileA.starts_with("a/"))
    {
        fileA.remove_prefix(2);
    }

    auto fileB = std::string_view{};
    if (match[3].matched)
    {
        fileB = string_viewIteratorToString_view(match[2].first, match[2].second);
        if (fileB.starts_with("a/"))
        {
            fileB.remove_prefix(2);
        }
    }

    return DiffLine{ .isCombined = isCombined, .fileA = fileA, .fileB = fileB };
}

} // namespace CppGit

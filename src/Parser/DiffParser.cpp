#include "Parser/DiffParser.hpp"

#include <charconv>
#include <regex>

auto CppGit::DiffParser::parse(const std::string_view diffContent) -> std::vector<DiffFile>
{
    auto splittedDiff = split(diffContent, '\n');
    std::vector<HeaderLine> headersLines;

    std::vector<DiffFile> diffFiles;
    DiffFile diffFile;

    for (auto iterator = splittedDiff.cbegin(); iterator < splittedDiff.cend(); ++iterator)
    {
        auto line = *iterator;
        switch (currentState)
        {
        case ParseState::WAITING_FOR_DIFF: {
            if (line.substr(0, 4) != "diff")
            {
                throw std::runtime_error("Invalid diff format");
            }
            currentState = ParseState::HEADER;
            diffFile = DiffFile{};
            headersLines.clear();
            diffFile.isCombined = isCombinedDiff(line) ? DiffType::COMBINED : DiffType::NORMAL;
            break;
        }
        case ParseState::HEADER: {
            auto lastHeaderLineType = headersLines.empty() ? HeaderLineType::NO_LINE : headersLines.back().type;
            auto headerLine = parseHeaderLine(line, lastHeaderLineType);

            if (headerLine.type == HeaderLineType::END_HEADER)
            {
                if (line.substr(0, 4) == "diff")
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
                    auto [indicesBeforeSV, indexAfter, mode] = std::get<std::tuple<std::string_view, std::string_view, int>>(headerLine.value);
                    auto indicesBefore = split(indicesBeforeSV, ',');
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
                        diffFile.newMode = std::get<int>(headerLine.value);
                    }
                    else if (headerLine.type == HeaderLineType::DELETED_FILE)
                    {
                        diffFile.diffStatus = DiffStatus::DELETED;
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
                else if (diffFile.diffStatus == DiffStatus::RENAMED)
                {
                    diffFile.fileB = std::get<std::string_view>(headerLine.value);
                }
                else if (diffFile.diffStatus == DiffStatus::COPIED)
                {
                    diffFile.fileB = std::get<std::string_view>(headerLine.value);
                }

                headersLines.push_back(parseHeaderLine(line, lastHeaderLineType));
            }
            break;
        }
        case ParseState::HUNK_FILE_A: {
            auto fileA = split(line, ' ')[1];
            if (fileA.substr(0, 2) == "a/")
            {
                fileA.remove_prefix(2);
            }
            diffFile.fileA = fileA;
            currentState = ParseState::HUNK_FILE_B;
            break;
        }
        case ParseState::HUNK_FILE_B: {
            auto fileB = split(line, ' ')[1];
            if (fileB.substr(0, 2) == "b/")
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
            if (line.substr(0, 4) == "diff")
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

    diffFiles.push_back(std::move(diffFile));

    return diffFiles;
}


auto CppGit::DiffParser::isCombinedDiff(const std::string_view line) -> bool
{
    static constexpr auto pattern = R"(^diff --(\w+) .+$)";

    std::regex regex{ pattern };

    if (std::match_results<std::string_view::const_iterator> match; std::regex_match(line.begin(), line.end(), match, regex))
    {
        std::string_view captured_group(match[1].first, std::distance(match[1].first, match[1].second));

        return captured_group == "cc";
    }

    throw std::runtime_error("Invalid diff format");

    return false;
}

auto CppGit::DiffParser::parseHeaderLine(const std::string_view line, const HeaderLineType headerLineBefore) -> HeaderLine
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
            return HeaderLine{ HeaderLineType::INDEX, std::make_tuple(string_viewIteratorToString_view(match[1].first, match[1].second), string_viewIteratorToString_view(match[2].first, match[2].second), getIntFromStringViewMatch(match, 3)) };
        }
        else if (std::regex_match(line.cbegin(), line.cend(), match, std::regex{ newFilePattern }))
        {
            return HeaderLine{ HeaderLineType::NEW_FILE, getIntFromStringViewMatch(match, 1) };
        }
        else if (std::regex_match(line.begin(), line.end(), match, std::regex{ deletedFilePattern }))
        {
            return HeaderLine{ HeaderLineType::DELETED_FILE, getIntFromStringViewMatch(match, 1) };
        }
        else if (std::regex_match(line.begin(), line.end(), match, std::regex{ similarityIndexPattern }))
        {
            return HeaderLine{ HeaderLineType::SIMILARITY_INDEX, getIntFromStringViewMatch(match, 1) };
        }
        else if (std::regex_match(line.cbegin(), line.cend(), match, std::regex{ oldModePattern }))
        {
            return HeaderLine{ HeaderLineType::OLD_MODE, getIntFromStringViewMatch(match, 1) };
        }
        break;

    case HeaderLineType::SIMILARITY_INDEX:
        if (std::regex_match(line.cbegin(), line.cend(), match, std::regex{ renamedFromPattern }))
        {
            return HeaderLine{ HeaderLineType::RENAME_FROM, string_viewIteratorToString_view(match[1].first, match[1].second) };
        }
        else if (std::regex_match(line.cbegin(), line.cend(), match, std::regex{ copyFromPattern }))
        {
            return HeaderLine{ HeaderLineType::COPY_FROM, string_viewIteratorToString_view(match[1].first, match[1].second) };
        }
        break;

    case HeaderLineType::RENAME_FROM:
        std::regex_match(line.cbegin(), line.cend(), match, std::regex{ renamedToPattern });

        return HeaderLine{ HeaderLineType::RENAME_TO, string_viewIteratorToString_view(match[1].first, match[1].second) };

    case HeaderLineType::COPY_FROM:
        std::regex_match(line.cbegin(), line.cend(), match, std::regex{ copyToPattern });

        return HeaderLine{ HeaderLineType::COPY_TO, string_viewIteratorToString_view(match[1].first, match[1].second) };

    case HeaderLineType::OLD_MODE: {
        std::regex_match(line.cbegin(), line.cend(), match, std::regex{ newModePattern });

        return HeaderLine{ HeaderLineType::NEW_MODE, getIntFromStringViewMatch(match, 1) };
    }
    case HeaderLineType::RENAME_TO:
    case HeaderLineType::COPY_TO:
    case HeaderLineType::NEW_MODE:
        if (std::regex_match(line.cbegin(), line.cend(), match, std::regex{ indexPattern }))
        {
            return HeaderLine{ HeaderLineType::INDEX, std::make_tuple(string_viewIteratorToString_view(match[1].first, match[1].second), string_viewIteratorToString_view(match[2].first, match[2].second), getIntFromStringViewMatch(match, 3)) };
        }
        return HeaderLine{ HeaderLineType::END_HEADER, 0 };

    case HeaderLineType::NEW_FILE:
    case HeaderLineType::DELETED_FILE: {
        std::regex_match(line.cbegin(), line.cend(), match, std::regex{ indexPattern });

        return HeaderLine{ HeaderLineType::INDEX, std::make_tuple(string_viewIteratorToString_view(match[1].first, match[1].second), string_viewIteratorToString_view(match[2].first, match[2].second), getIntFromStringViewMatch(match, 3)) };
    }
    case HeaderLineType::INDEX:
        return HeaderLine{ HeaderLineType::END_HEADER, 0 };

    case HeaderLineType::END_HEADER:
        break;
    }

    return HeaderLine();
}

auto CppGit::DiffParser::getIntFromStringViewMatch(const std::match_results<std::string_view::const_iterator>& match, std::size_t index) -> int
{
    int mode{ 0 };
    if (match[index].first != match[index].second)
    {
        std::from_chars(match[index].first, match[index].second, mode);
    }

    return mode;
}

auto CppGit::DiffParser::parseHunkHeader(const std::string_view line) -> std::pair<std::vector<std::pair<int, int>>, std::pair<int, int>>
{
    static constexpr auto hunkHeaderPattern = R"(^@{2,} ((?:-\d+(?:,\d+)?\s)+)(\+\d+(?:,\d+)?) @{2,}$)";

    auto match = std::match_results<std::string_view::const_iterator>();
    std::regex_match(line.cbegin(), line.cend(), match, std::regex{ hunkHeaderPattern });
    auto beforeIndicesSV = string_viewIteratorToString_view(match[1].first, match[1].second - 1);
    auto beforeIndices = split(beforeIndicesSV, ' ');

    auto hunkRangesBefore = std::vector<std::pair<int, int>>{};
    auto hunkRangeAfter = std::pair<int, int>{};

    for (auto beforeRange : beforeIndices)
    {
        auto splittedRange = split(beforeRange, ',');
        int left{ 0 };
        int right{ -1 };
        std::from_chars(splittedRange[0].data() + 1, splittedRange[0].data() + splittedRange[0].size(), left);
        if (splittedRange.size() == 2)
        {
            std::from_chars(splittedRange[1].data(), splittedRange[1].data() + splittedRange[1].size(), right);
        }

        hunkRangesBefore.emplace_back(left, right);
    }

    auto splittedRange = split(string_viewIteratorToString_view(match[2].first, match[2].second), ',');
    int left{ 0 };
    int right{ -1 };
    std::from_chars(splittedRange[0].data() + 1, splittedRange[0].data() + splittedRange[0].size(), left);
    if (splittedRange.size() == 2)
    {
        std::from_chars(splittedRange[1].data(), splittedRange[1].data() + splittedRange[1].size(), right);
    }

    hunkRangeAfter = std::make_pair(left, right);

    return std::make_pair(hunkRangesBefore, hunkRangeAfter);
}

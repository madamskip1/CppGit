#include "Parser/DiffParser.hpp"

#include <charconv>
#include <regex>

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

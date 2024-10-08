#pragma once

#include "Parser.hpp"

#include <regex>
#include <string>
#include <variant>
#include <vector>

enum class DiffStatus
{
    ADDED,
    COPIED,
    DELETED,
    MODDIFIED,
    RENAMED,
    TYPE_CHANGED,
    UNKNOWN
};

struct DiffHunk
{
    std::pair<int, int> rangeBefore;
    std::pair<int, int> rangeAfter;
    std::vector<std::string> content;
};

struct DiffFile
{
    std::string fileA;
    std::string fileB;
    std::vector<std::string> indicesBefore;
    std::string indexesAfter;
};

namespace CppGit {

class DiffParser : protected Parser
{
public:
    enum class ParseState
    {
        WAITING_FOR_DIFF,
        HEADER,
        EXTENDED_HEADER,
        HUNK_FILES,
        HUNK_HEADER,
        HUNK_CONTENT
    };

    enum class DiffType
    {
        NORMAL,
        COMBINED
    };

    enum class HeaderLineType
    {
        NO_LINE,
        OLD_MODE,
        NEW_MODE,
        DELETED_FILE,
        NEW_FILE,
        COPY_FROM,
        COPY_TO,
        RENAME_FROM,
        RENAME_TO,
        SIMILARITY_INDEX,
        INDEX,
        END_HEADER
    };

    struct HeaderLine
    {
        HeaderLineType type;
        std::variant<int, std::string_view, std::tuple<std::string_view, std::string_view, int>> value;
    };

    ParseState currentState;

    static auto parseHeaderLine(const std::string_view line, const HeaderLineType headerLineBefore) -> HeaderLine;

private:
    static auto getIntFromStringViewMatch(const std::match_results<std::string_view::const_iterator>& match, std::size_t index) -> int;
};

} // namespace CppGit

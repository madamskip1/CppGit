#pragma once

#include "Parser.hpp"

#include <regex>
#include <string>
#include <variant>
#include <vector>

namespace CppGit {

enum class DiffStatus
{
    UNKNOWN,
    NEW,
    COPIED,
    COPIED_AND_MODIFIED,
    DELETED,
    MODDIFIED,
    RENAMED,
    RENAMED_AND_MODIFIED,
    TYPE_CHANGED,
    TYPE_CHANGED_SYMLINK,
    BINARY_CHANGED
};

enum class DiffType
{
    NORMAL,
    COMBINED
};
struct DiffFile
{
    DiffType isCombined;
    DiffStatus diffStatus{ DiffStatus::UNKNOWN };

    std::string fileA;
    std::string fileB;
    std::vector<std::string> indicesBefore;
    std::string indexAfter;
    int oldMode{ 0 };
    int newMode{ 0 };
    int similarityIndex{ 0 };

    std::vector<std::pair<int, int>> hunkRangesBefore;
    std::pair<int, int> hunkRangeAfter;
    std::vector<std::string> hunkContent;
};
class DiffParser : protected Parser
{
public:
    DiffParser() = default;

    auto parse(const std::string_view diffContent) -> std::vector<DiffFile>;

    // private:
    enum class ParseState
    {
        WAITING_FOR_DIFF,
        HEADER,
        HUNK_FILE_A,
        HUNK_FILE_B,
        HUNK_HEADER,
        HUNK_CONTENT
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

    static auto isCombinedDiff(const std::string_view line) -> bool;
    static auto parseHeaderLine(const std::string_view line, const HeaderLineType headerLineBefore) -> HeaderLine;

private:
    static auto getIntFromStringViewMatch(const std::match_results<std::string_view::const_iterator>& match, std::size_t index) -> int;
    static auto parseHunkHeader(const std::string_view line) -> std::pair<std::vector<std::pair<int, int>>, std::pair<int, int>>;
    static auto parseHunkHeaderRange(const std::string_view range) -> std::pair<int, int>;
};


} // namespace CppGit

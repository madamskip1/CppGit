#pragma once

#include "DiffFile.hpp"
#include "Parser.hpp"

#include <regex>
#include <variant>
#include <vector>

namespace CppGit {

class DiffParser : protected Parser
{
public:
    DiffParser() = default;

    auto parse(const std::string_view diffContent) -> std::vector<DiffFile>;

private:
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

    struct DiffLine
    {
        bool isCombined;
        std::string_view fileA;
        std::string_view fileB;
    };

    ParseState currentState{ ParseState::WAITING_FOR_DIFF };

    static auto parseHeaderLine(const std::string_view line, const HeaderLineType headerLineBefore) -> HeaderLine;
    static auto parseHunkHeader(const std::string_view line) -> std::pair<std::vector<std::pair<int, int>>, std::pair<int, int>>;
    static auto parseHunkHeaderRange(const std::string_view range) -> std::pair<int, int>;
    static auto parseDiffLine(const std::string_view line) -> DiffLine;
    static auto getIntFromStringViewMatch(const std::match_results<std::string_view::const_iterator>& match, std::size_t index) -> int;
};


} // namespace CppGit

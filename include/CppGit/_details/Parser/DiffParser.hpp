#pragma once

#include "../../DiffFile.hpp"
#include "Parser.hpp"

#include <cstdint>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

namespace CppGit {

/// @brief Provides internal functionality to parse diff content
class DiffParser final : protected Parser
{
public:
    /// @brief Parse the diff content
    /// @param diffContent Diff content
    /// @return Parsed diff files
    [[nodiscard]] auto parse(const std::string_view diffContent) -> std::vector<DiffFile>;

private:
    enum class ParseState : uint8_t
    {
        WAITING_FOR_DIFF,
        HEADER,
        BINARY_FILE,
        HUNK_FILE_A,
        HUNK_FILE_B,
        HUNK_HEADER,
        HUNK_CONTENT
    };

    enum class HeaderLineType : uint8_t
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
    static auto parseHunkFileLine(const std::string_view line, const std::string_view prefix) -> std::string;
    static auto parseHunkHeader(const std::string_view line) -> std::pair<std::vector<std::pair<int, int>>, std::pair<int, int>>;
    static auto parseHunkHeaderRange(const std::string_view range) -> std::pair<int, int>;
    static auto parseDiffLine(const std::string_view line) -> DiffLine;

    static auto processHeaderLine(const HeaderLine& headerLine, DiffFile& diffFile) -> void;

    static auto peakNextLine(std::vector<std::string_view>::const_iterator iterator, const std::vector<std::string_view>::const_iterator endIterator) -> std::string_view;

    static auto removePrefixFromFileIfStartsWith(std::string_view& file, const std::string_view prefix) -> void;
};

} // namespace CppGit

#pragma once

#include "Index.hpp"
#include "Parser.hpp"

#include <string_view>
#include <vector>

namespace CppGit {
class IndexParser final : protected Parser
{
public:
    /// @brief Parse index stage detailed entry
    /// @param indexEntryLine Index entry line
    /// @return Index entry
    static auto parseStageDetailedEntry(const std::string_view indexEntryLine) -> IndexEntry;

    /// @brief Parse index stage detailed list
    /// @param indexContent Index stage content
    /// @return List of index entries
    static auto parseStageDetailedList(const std::string_view indexContent) -> std::vector<IndexEntry>;

    /// @brief Parse index cache list
    /// @param indexContent Index cache content
    /// @return List of index cache entries
    static auto parseStageSimpleCacheList(const std::string_view indexContent) -> std::vector<std::string>;

    /// @brief Parse diff index entry
    /// @param diffIndexLine Single diff index line
    /// @return Diff index entry
    static auto parseDiffIndexWithStatusEntry(const std::string_view diffIndexLine) -> DiffIndexEntry;

    /// @brief Parse diff index
    /// @param diffIndexContent Diff index content
    /// @return List of diff index entries
    static auto parseDiffIndexWithStatus(const std::string_view diffIndexContent) -> std::vector<DiffIndexEntry>;

    /// @brief Parse ls-files entry
    /// @param lsFilesLine Single ls-files line
    /// @return Ls-files entry
    static auto parseLsFilesEntry(const std::string_view lsFilesLine) -> LsFilesEntry;

    /// @brief Parse ls-files list
    /// @param lsFilesContent Ls-files content
    /// @return List of ls-files entries
    static auto parseLsFilesList(const std::string_view lsFilesContent) -> std::vector<LsFilesEntry>;

private:
    static auto getDiffIndexStatus(const std::string_view status) -> DiffIndexStatus;
    static auto getLsFilesStatus(const std::string_view status) -> LsFilesStatus;
};

} // namespace CppGit

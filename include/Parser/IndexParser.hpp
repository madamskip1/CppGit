#pragma once

#include "../Index.hpp"
#include "Parser.hpp"

#include <string_view>
#include <vector>

namespace CppGit {
class IndexParser : protected Parser
{
public:
    static auto parseStageDetailedEntry(const std::string_view indexEntryLine) -> IndexEntry;
    static auto parseStageDetailedList(const std::string_view indexContent) -> std::vector<IndexEntry>;
    static auto parseStageSimpleCacheList(const std::string_view indexContent) -> std::vector<std::string>;

    static auto getDiffIndexStatus(const std::string_view status) -> DiffIndexStatus;
    static auto parseDiffIndexWithStatusEntry(const std::string_view diffIndexLine) -> DiffIndexEntry;
    static auto parseDiffIndexWithStatus(const std::string_view diffIndexContent) -> std::vector<DiffIndexEntry>;

    static auto getLsFilesStatus(const std::string_view status) -> LsFilesStatus;
    static auto parseLsFilesEntry(const std::string_view lsFilesLine) -> LsFilesEntry;
    static auto parseLsFilesList(const std::string_view lsFilesContent) -> std::vector<LsFilesEntry>;
};

} // namespace CppGit

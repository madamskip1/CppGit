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
};

} // namespace CppGit

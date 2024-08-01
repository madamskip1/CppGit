#pragma once

#include "../Index.hpp"
#include "Parser.hpp"

#include <string_view>
#include <vector>

namespace CppGit {
class IndexParser : protected Parser
{
public:
    static IndexEntry parseStageDetailedEntry(const std::string_view indexEntryLine);
    static std::vector<IndexEntry> parseStageDetailedList(const std::string_view indexContent);
};

} // namespace CppGit

#pragma once

#include <string>
#include <utility>
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

} // namespace CppGit

#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace CppGit {

/// @brief Enum class to represent the status of a file in a diff
enum class DiffStatus : uint8_t
{
    UNKNOWN,              ///< Unknown status
    NEW,                  ///< New file
    COPIED,               ///< Copied file
    COPIED_AND_MODIFIED,  ///< Copied and modified content of file
    DELETED,              ///< Deleted file
    MODDIFIED,            ///< Modified content of file
    RENAMED,              ///< Renamed file
    RENAMED_AND_MODIFIED, ///< Renamed and modified content of file
    TYPE_CHANGED,         ///< Type of file changed
    TYPE_CHANGED_SYMLINK, ///< Type of file changed to symlink
    BINARY_CHANGED        ///< Binary file changed
};

/// @brief A file in a diff
struct DiffFile
{
    bool isCombined{ false };                          ///< True if the file is a combined diff (for example, a merge commit)
    DiffStatus diffStatus{ DiffStatus::UNKNOWN };      ///< Status of the file

    std::string fileA;                                 ///< File before the change
    std::string fileB;                                 ///< File after the change
    std::vector<std::string> indicesBefore;            ///< Line indices before the change
    std::string indexAfter;                            ///< Line index after the change
    int oldMode{ 0 };                                  ///< File mode before the change
    int newMode{ 0 };                                  ///< File mode after the change
    int similarityIndex{ 0 };                          ///< Similarity between the files

    std::vector<std::pair<int, int>> hunkRangesBefore; ///< Ranges of hunks before the change
    std::pair<int, int> hunkRangeAfter;                ///< Range of hunks after the change
    std::vector<std::string> hunkContent;              ///< Content of the hunks
};

} // namespace CppGit

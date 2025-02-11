#pragma once

#include <cstdint>

namespace CppGit {

/// @brief Enum class to represent errors in CppGit library
enum class Error : uint8_t
{
    NO_ERROR,                       ///< No error
    DIRTY_WORKTREE,                 ///< Working tree is dirty
    PATTERN_NOT_MATCHING_ANY_FILES, ///< Pattern does not match any files

    // Cherry-pick
    NO_CHERRY_PICK_IN_PROGRESS, ///< No cherry-pick in progress
    CHERRY_PICK_CONFLICT,       ///< Conflict during cherry-pick
    CHERRY_PICK_EMPTY_COMMIT,   ///< Empty commit during cherry-pick

    // Rebase
    REBASE_CONFLICT,       ///< Conflict during rebase
    NO_REBASE_IN_PROGRESS, ///< No rebase in progress
    REBASE_BREAK,          ///<  Stop at break during rebase
    REBASE_REWORD,         ///< Stop at reword during rebase
    REBASE_SQUASH,         ///<  Stop at squash during rebase
    REBASE_EDIT,           ///< Stopped at edit during rebase

    // MERGE
    MERGE_NOTHING_TO_MERGE,       ///< Nothing to merge
    MERGE_FF_BRANCHES_DIVERGENCE, ///< Branches have diverged
    MERGE_NO_FF_CONFLICT,         ///< Conflict during non-fast-forward merge
    NO_MERGE_IN_PROGRESS,         ///< No merge in progress

    // Repository
    CLONE_NO_PATH_GIVEN,          ///< No path given for clone
    PATH_IS_NOT_A_DIRECTORY,      ///< Path is not a directory
    PATH_DIR_IS_NOT_EMPTY,        ///< Directory is not empty
    FAILED_TO_CREATE_DIRECTORIES, ///< Failed to create directories
    CLONE_FAILED,                 ///< Clone failed
};

} // namespace CppGit

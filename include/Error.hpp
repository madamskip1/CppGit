#pragma once

namespace CppGit {

enum class Error
{
    NONE,
    DIRTY_WORKTREE,
    PATTERN_NOT_MATCHING_ANY_FILES,

    // Cherry-pick
    NO_CHERRY_PICK_IN_PROGRESS,
    CHERRY_PICK_CONFLICT,
    CHERRY_PICK_EMPTY_COMMIT,

    // Rebase
    REBASE_CONFLICT,
    NO_REBASE_IN_PROGRESS,
};

} // namespace CppGit

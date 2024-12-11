#pragma once

namespace CppGit {

enum class Error
{
    NONE,
    DIRTY_WORKTREE,

    // Cherry-pick
    NO_CHERRY_PICK_IN_PROGRESS,
    CHERRY_PICK_CONFLICT,
    CHERRY_PICK_EMPTY_COMMIT
};

} // namespace CppGit

#pragma once

#include <cstdint>

namespace CppGit {

/// @brief Enum class to represent errors in CppGit library
enum class Error : uint8_t
{
    NO_ERROR, ///< No error

    // Cherry-pick
    NO_CHERRY_PICK_IN_PROGRESS,                ///< No cherry-pick in progress
    CHERRY_PICK_CONFLICT,                      ///< Conflict during cherry-pick
    CHERRY_PICK_EMPTY_COMMIT,                  ///< Empty commit during cherry-pick
    CHERRY_PICK_UNKNOWN_EMPTY_COMMIT_STRATEGY, ///< Unknown empty commit strategy during cherry-pick
};

} // namespace CppGit

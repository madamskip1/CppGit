#pragma once

namespace CppGit {

enum class ErrorCode
{
    NO_ERROR = 0,
    GIT_CLONE_NO_PATH_GIVEN = -1,
    GIT_CLONE_PATH_IS_NOT_A_DIRECTORY = -2,
    GIT_CLONE_PATH_DIR_IS_NOT_EMPTY = -3,
    GIT_CLONE_FAILED_TO_CREATE_DIRECTORIES = -4,
    GIT_CLONE_FAILED = -5
};

} // namespace CppGit

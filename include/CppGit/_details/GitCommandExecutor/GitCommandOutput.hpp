#pragma once

#include <string>

namespace CppGit {

/// @brief Represents the output of a git command
struct GitCommandOutput
{
    int return_code;    ///< The return code of the command
    std::string stdout; ///< The standard output of the command
    std::string stderr; ///< The standard error of the command
};

} // namespace CppGit

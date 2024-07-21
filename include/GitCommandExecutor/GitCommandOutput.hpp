#pragma once

#include <string>

namespace CppGit {

struct GitCommandOutput
{
    int return_code;
    std::string stdout;
    std::string stderr;
};

} // namespace CppGit

#pragma once

#include <exception>

namespace CppGit {

class MergeConflict : public std::exception
{ };

} // namespace CppGit

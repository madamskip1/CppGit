#pragma once

#include "Repository.hpp"

#include <filesystem>

namespace CppGit {

struct IndexEntry
{
    std::string fileMode;
    std::string objectHash;
    int stageNumber;
    std::filesystem::path path;
};


} // namespace CppGit

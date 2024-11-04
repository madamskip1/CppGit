#pragma once

#include "Repository.hpp"

#include <string_view>

namespace CppGit {


class CherryPick
{
public:
    explicit CherryPick(const Repository& repo);

    auto cherryPickCommit(const std::string_view commitHash) const -> std::string;

private:
    const Repository& repo;
};

} // namespace CppGit

#pragma once

#include "Repository.hpp"

namespace CppGit {

class Rebase
{

public:
    explicit Rebase(const Repository& repo);

    auto rebase(const std::string_view upstream) const -> std::string;

private:
    const Repository& repo;
};

} // namespace CppGit

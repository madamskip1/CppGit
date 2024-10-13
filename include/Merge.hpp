#pragma once

#include <string_view>
#include "Repository.hpp"

namespace CppGit {

class Merge
{
public:
    explicit Merge(const Repository& repo)
        : repo(repo)
    { }

    auto canFastForward(const std::string_view sourceBranch) const -> bool;
    auto canFastForward(const std::string_view sourceBranch, const std::string_view targetBranch) const -> bool;

    auto isAnythingToMerge(const std::string_view sourceBranch) const -> bool;
    auto isAnythingToMerge(const std::string_view sourceBranch, const std::string_view targetBranch) const -> bool;

private:
    const Repository& repo;

    auto getAncestor(const std::string_view sourceBranch, const std::string_view targetBranch) const -> std::string;
};

} // namespace CppGit

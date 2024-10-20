#pragma once

#include <string>
#include <string_view>
#include "Repository.hpp"

namespace CppGit {

class Merge
{
public:
    explicit Merge(const Repository& repo)
        : repo(repo)
    { }

    auto mergeFastForward(const std::string_view sourceBranch) const -> std::string;
    auto mergeFastForward(const std::string_view sourceBranch, const std::string_view targetBranch) const -> std::string;

    auto mergeNoFastForward(const std::string_view sourceBranch, const std::string_view message, const std::string_view description = "") const -> std::string;

    auto canFastForward(const std::string_view sourceBranch) const -> bool;
    auto canFastForward(const std::string_view sourceBranch, const std::string_view targetBranch) const -> bool;

    auto isAnythingToMerge(const std::string_view sourceBranch) const -> bool;
    auto isAnythingToMerge(const std::string_view sourceBranch, const std::string_view targetBranch) const -> bool;

private:
    const Repository& repo;

    auto getAncestor(const std::string_view sourceBranch, const std::string_view targetBranch) const -> std::string;
    auto createMergeCommit(const std::string_view sourceBranchRef, const std::string_view targetBranchRef, const std::string_view message, const std::string_view description) const -> std::string;
};

} // namespace CppGit

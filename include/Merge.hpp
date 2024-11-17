#pragma once

#include "Index.hpp"
#include "Repository.hpp"
#include "_details/CreateCommit.hpp"
#include "_details/ThreeWayMerge.hpp"

#include <string>
#include <string_view>
#include <unordered_map>

namespace CppGit {

class Merge
{
public:
    explicit Merge(const Repository& repo);

    auto mergeFastForward(const std::string_view sourceBranch) const -> std::string;
    auto mergeFastForward(const std::string_view sourceBranch, const std::string_view targetBranch) const -> std::string;

    auto mergeNoFastForward(const std::string_view sourceBranch, const std::string_view message, const std::string_view description = "") -> std::string;

    auto canFastForward(const std::string_view sourceBranch) const -> bool;
    auto canFastForward(const std::string_view sourceBranch, const std::string_view targetBranch) const -> bool;

    auto isAnythingToMerge(const std::string_view sourceBranch) const -> bool;
    auto isAnythingToMerge(const std::string_view sourceBranch, const std::string_view targetBranch) const -> bool;

    auto isMergeInProgress() const -> bool;
    auto isThereAnyConflict() const -> bool;

    auto abortMerge() const -> void;
    auto continueMerge() const -> std::string;

private:
    const Repository& repo;
    const _details::CreateCommit _createCommit;
    const _details::ThreeWayMerge _threeWayMerge;

    std::string mergeInProgress_sourceBranchRef;
    std::string mergeInProgress_targetBranchRef;

    auto getAncestor(const std::string_view sourceBranch, const std::string_view targetBranch) const -> std::string;
    auto createMergeCommit(const std::string_view sourceBranchRef, const std::string_view targetBranchRef, const std::string_view message, const std::string_view description) const -> std::string;
    auto startMergeConflict(const std::vector<IndexEntry>& unmergedFilesEntries, const std::string_view sourceBranchRef, const std::string_view sourceLabel, const std::string_view targetBranchRef, const std::string_view targetLabel, const std::string_view message, const std::string_view description) -> void;

    auto createNoFFMergeFiles(const std::string_view sourceBranchRef, const std::string_view message, const std::string_view description) const -> void;
    auto removeNoFFMergeFiles() const -> void;
};

} // namespace CppGit

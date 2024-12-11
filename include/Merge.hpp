#pragma once

#include "Error.hpp"
#include "Index.hpp"
#include "Repository.hpp"
#include "_details/CreateCommit.hpp"
#include "_details/IndexWorktree.hpp"
#include "_details/ThreeWayMerge.hpp"

#include <expected>
#include <string>
#include <string_view>

namespace CppGit {

class Merge
{
public:
    explicit Merge(const Repository& repo);

    auto mergeFastForward(const std::string_view sourceBranch) const -> std::expected<std::string, Error>;
    auto mergeFastForward(const std::string_view sourceBranch, const std::string_view targetBranch) const -> std::expected<std::string, Error>;

    auto mergeNoFastForward(const std::string_view sourceBranch, const std::string_view message, const std::string_view description = "") -> std::expected<std::string, Error>;

    auto canFastForward(const std::string_view sourceBranch) const -> bool;
    auto canFastForward(const std::string_view sourceBranch, const std::string_view targetBranch) const -> bool;

    auto isAnythingToMerge(const std::string_view sourceBranch) const -> bool;
    auto isAnythingToMerge(const std::string_view sourceBranch, const std::string_view targetBranch) const -> bool;

    auto isMergeInProgress() const -> bool;
    auto isThereAnyConflict() const -> std::expected<bool, Error>;

    auto abortMerge() const -> Error;
    auto continueMerge() const -> std::expected<std::string, Error>;

private:
    const Repository& repo;
    const _details::CreateCommit _createCommit;
    const _details::ThreeWayMerge _threeWayMerge;
    const _details::IndexWorktree _indexWorktree;

    std::string mergeInProgress_sourceBranchRef;
    std::string mergeInProgress_targetBranchRef;

    auto getAncestor(const std::string_view sourceBranch, const std::string_view targetBranch) const -> std::string;
    auto createMergeCommit(const std::string_view sourceBranchRef, const std::string_view targetBranchRef, const std::string_view message, const std::string_view description) const -> std::string;
    auto startMergeConflict(const std::vector<IndexEntry>& unmergedFilesEntries, const std::string_view sourceBranchRef, const std::string_view sourceLabel, const std::string_view targetBranchRef, const std::string_view targetLabel, const std::string_view message, const std::string_view description) -> void;

    auto createNoFFMergeFiles(const std::string_view sourceBranchRef, const std::string_view message, const std::string_view description) const -> void;
    auto removeNoFFMergeFiles() const -> void;

    auto isThereAnyConflictImpl() const -> bool;
};

} // namespace CppGit

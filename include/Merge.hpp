#pragma once

#include "Index.hpp"

#include <string>
#include <string_view>
#include "Repository.hpp"
#include <unordered_map>

namespace CppGit {

class Merge
{
public:
    explicit Merge(const Repository& repo)
        : repo(repo)
    { }

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
    struct UnmergedFileBlobs
    {
        std::string baseBlob;   // merge-base
        std::string targetBlob; // ours
        std::string sourceBlob; // their
    };

    const Repository& repo;

    std::string mergeInProgress_sourceBranchRef;
    std::string mergeInProgress_targetBranchRef;
    std::string mergeInProgress_message;
    std::string mergeInProgress_description;

    auto getAncestor(const std::string_view sourceBranch, const std::string_view targetBranch) const -> std::string;
    auto createMergeCommit(const std::string_view sourceBranchRef, const std::string_view targetBranchRef, const std::string_view message, const std::string_view description) const -> std::string;
    auto startMergeConflict(const std::string_view sourceBranchRef, const std::string_view sourceLabel, const std::string_view targetBranchRef, const std::string_view targetLabel, const std::string_view message, const std::string_view description) -> void;
    auto unpackFile(const std::string_view fileBlob) const -> std::string;
    auto parseUnmergedFiles(const std::vector<CppGit::IndexEntry>& indexEntries) const -> std::unordered_map<std::string, UnmergedFileBlobs>;
    auto createNoFFMergeFiles(const std::string_view sourceBranchRef, const std::string_view message, const std::string_view description) const -> void;
    auto removeNoFFMergeFiles() const -> void;
};

} // namespace CppGit

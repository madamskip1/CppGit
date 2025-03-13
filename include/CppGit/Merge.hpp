#pragma once

#include "Index.hpp"
#include "Repository.hpp"
#include "_details/GitFilesHelper.hpp"
#include "_details/IndexWorktree.hpp"
#include "_details/ThreeWayMerge.hpp"

#include <cstdint>
#include <expected>
#include <string>
#include <string_view>

namespace CppGit {

/// @brief Result of a merge operation
enum class MergeResult : uint8_t
{
    NOTHING_TO_MERGE,       ///< Nothing to merge
    FF_BRANCHES_DIVERGENCE, ///< Branches have diverged
    NO_FF_CONFLICT,         ///< Conflict during non-fast-forward merge
};

/// @brief Provides functionality to merge branches
class Merge
{
public:
    /// @param repo The repository to work with
    explicit Merge(const Repository& repo);

    /// @brief Merge source branch onto current branch using fast-forward merge
    ///    Will fail if merge fast-forward is not possible
    /// @param sourceBranch Source branch name to merge
    /// @return Source branch's commit hash if merge FF is successful, otherwise Merge Result error code
    auto mergeFastForward(const std::string_view sourceBranch) const -> std::expected<std::string, MergeResult>;

    /// @brief Merge source branch onto target branch using fast-forward
    ///     Will fail if merge fast-forward is not possible
    /// @param sourceBranch Source branch name to merge
    /// @param targetBranch Target branch name to merge onto
    /// @return Source branch's commit hash if merge FF is successful, otherwise Merge Result error code
    auto mergeFastForward(const std::string_view sourceBranch, const std::string_view targetBranch) const -> std::expected<std::string, MergeResult>;

    /// @brief Merge source branch onto current branch using no-fast-forward merge
    /// @param sourceBranch Source branch name to merge
    /// @param message Merge commit message
    /// @param description Merge commit description (optional)
    /// @return Merge commit hash if merge No-FF is successful, otherwise Merge Result error code
    auto mergeNoFastForward(const std::string_view sourceBranch, const std::string_view message, const std::string_view description = "") const -> std::expected<std::string, MergeResult>;

    /// @brief Check whether fast-forward merge source branch onto current branch is possible
    /// @param sourceBranch Source branch name that would be merged
    /// @return True if fast-forward merge is possible, otherwise false
    [[nodiscard]] auto canFastForward(const std::string_view sourceBranch) const -> bool;

    /// @brief Check whether fast-forward merge source branch onto target branch is possible
    /// @param sourceBranch Source branch name that would be merged
    /// @param targetBranch Target branch name that would be merged onto
    /// @return True if fast-forward merge is possible, otherwise false
    [[nodiscard]] auto canFastForward(const std::string_view sourceBranch, const std::string_view targetBranch) const -> bool;

    /// @brief Check whether there is anything to merge from source branch onto current branch
    /// @param sourceBranch Source branch name that would be merged
    /// @return True if there is anything to merge, otherwise false
    [[nodiscard]] auto isAnythingToMerge(const std::string_view sourceBranch) const -> bool;

    /// @brief Check whether there is anything to merge from source branch onto target branch
    /// @param sourceBranch Source branch name that would be merged
    /// @param targetBranch Target branch name that would be merged onto
    /// @return True if there is anything to merge, otherwise false
    [[nodiscard]] auto isAnythingToMerge(const std::string_view sourceBranch, const std::string_view targetBranch) const -> bool;

    /// @brief Check whether there is a merge in progress
    /// @return True if there is a merge in progress, otherwise false
    [[nodiscard]] auto isMergeInProgress() const -> bool;

    /// @brief Check whether there is any conflict in current merge
    /// @return True if there is any conflict, otherwise false
    [[nodiscard]] auto isThereAnyConflict() const -> bool;

    /// @brief Abort current merge in progress
    auto abortMerge() const -> void;

    /// @brief Continue current merge in progress
    /// @return Merge commit hash if merge is successful, otherwise Merge Result error code
    auto continueMerge() const -> std::expected<std::string, MergeResult>;

private:
    const Repository* repo;

    _details::ThreeWayMerge threeWayMerge;
    _details::IndexWorktree indexWorktree;
    _details::GitFilesHelper gitFilesHelper;

    auto getAncestor(const std::string_view sourceBranch, const std::string_view targetBranch) const -> std::string;
    auto createMergeCommit(const std::string_view sourceBranchRef, const std::string_view targetBranchRef, const std::string_view message, const std::string_view description) const -> std::string;
    auto startMergeConflict(const std::vector<IndexEntry>& unmergedFilesEntries, const std::string_view sourceBranchRef, const std::string_view sourceLabel, const std::string_view targetLabel, const std::string_view message, const std::string_view description) const -> void;

    auto createNoFFMergeFiles(const std::string_view sourceBranchRef, const std::string_view message, const std::string_view description) const -> void;
    auto removeNoFFMergeFiles() const -> void;

    auto isThereAnyConflictImpl() const -> bool;
};

} // namespace CppGit

#pragma once

#include "Repository.hpp"

#include <cstdint>
#include <expected>
#include <string>
#include <string_view>

namespace CppGit {

/// @brief Result of a cherry pick operation
enum class CherryPickResult : uint8_t
{
    CONFLICT,                   ///< Conflict during cherry-pick
    EMPTY_COMMIT_OR_EMPTY_DIFF, ///< Empty commit during cherry-pick
};

/// @brief Strategy to use when cherry picking an empty commit
enum class CherryPickEmptyCommitStrategy : uint8_t
{
    STOP, ///< Stop cherry picking, when an empty commit is encountered. CherryPicking will be stopped and the user will have to take action
    DROP, ///< Drop the empty commit
    KEEP  ///< Keep the empty commit
};

/// @brief Provides functionality to cherry pick commits
class CherryPicker
{
public:
    /// @param repo The repository to work with
    explicit CherryPicker(const Repository& repository);

    /// @brief Cherry pick a commit
    /// @param commitHash The commit hash to cherry pick
    /// @param emptyCommitStrategy The strategy to use when cherry picking an empty commit
    /// @return The hash of the cherry picked commit or Cherry Pick Result error code
    auto cherryPick(const std::string_view commitHash, const CherryPickEmptyCommitStrategy emptyCommitStrategy = CherryPickEmptyCommitStrategy::STOP) const -> std::expected<std::string, CherryPickResult>;

    /// @brief Continue cherry picking after stopping on an empty commit
    /// @return The hash of the cherry picked commit or Cherry Pick Result error code
    auto commitEmptyCherryPickedCommit() const -> std::expected<std::string, CherryPickResult>;

    /// @brief Continue cherry picking after stopping on a conflict
    /// @return The hash of the cherry picked commit or Cherry Pick Result error code
    auto continueCherryPick() const -> std::expected<std::string, CherryPickResult>;

    /// @brief Abort the cherry pick in progress
    auto abortCherryPick() const -> void;

    /// @brief Check whether a cherry pick is in progress
    /// @return True if a cherry pick is in progress, false otherwise
    [[nodiscard]] auto isCherryPickInProgress() const -> bool;

private:
    const Repository* repository;

    auto commitCherryPicked(const std::string_view commitHash) const -> std::string;
    auto processEmptyDiff(const std::string_view commitHash, const CherryPickEmptyCommitStrategy emptyCommitStrategy) const -> std::expected<std::string, CherryPickResult>;

    auto createCherryPickHeadFile(const std::string_view commitHash) const -> void;
    auto getCherryPickHead() const -> std::string;
    auto removeCherryPickHeadFile() const -> void;
};

} // namespace CppGit

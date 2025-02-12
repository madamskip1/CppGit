#pragma once

#include "Error.hpp"
#include "Repository.hpp"
#include "_details/ApplyDiff.hpp"
#include "_details/CreateCommit.hpp"

#include <cstdint>
#include <expected>
#include <string>
#include <string_view>

namespace CppGit {

enum class CherryPickEmptyCommitStrategy : uint8_t
{
    STOP,
    DROP,
    KEEP
};

class CherryPick
{
public:
    /// @param repo The repository to work with
    explicit CherryPick(const Repository& repo);

    /// @brief Cherry pick a commit
    /// @param commitHash The commit hash to cherry pick
    /// @param emptyCommitStrategy The strategy to use when cherry picking an empty commit
    /// @return The hash of the cherry picked commit or an error code
    auto cherryPickCommit(const std::string_view commitHash, CherryPickEmptyCommitStrategy emptyCommitStrategy = CherryPickEmptyCommitStrategy::STOP) const -> std::expected<std::string, Error>;

    /// @brief Continue cherry picking after stopping on an empty commit
    /// @return The hash of the cherry picked commit or an error code
    auto commitEmptyCherryPickedCommit() const -> std::expected<std::string, Error>;

    /// @brief Continue cherry picking after stopping on a conflict
    /// @return The hash of the cherry picked commit or an error code
    auto cherryPickContinue() const -> std::expected<std::string, Error>;

    /// @brief Check whether a cherry pick is in progress
    /// @return True if a cherry pick is in progress, false otherwise
    [[nodiscard]] auto isCherryPickInProgress() const -> bool;

private:
    const Repository& repo;
    const _details::CreateCommit _createCommit;
    const _details::ApplyDiff _applyDiff;


    auto commitCherryPicked(const std::string_view commitHash) const -> std::string;
    auto createCherryPickHeadFile(const std::string_view commitHash) const -> void;
    auto getCherryPickHead() const -> std::string;

    auto processEmptyDiff(const std::string_view commitHash, CherryPickEmptyCommitStrategy emptyCommitStrategy) const -> std::expected<std::string, Error>;
};

} // namespace CppGit

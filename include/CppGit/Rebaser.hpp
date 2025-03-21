#pragma once

#include "BranchesManager.hpp"
#include "CommitsManager.hpp"
#include "RebaseTodoCommand.hpp"
#include "Repository.hpp"
#include "_details/CommitAmender.hpp"
#include "_details/CommitCreator.hpp"
#include "_details/DiffApplier.hpp"
#include "_details/IndexWorktreeManager.hpp"
#include "_details/RebaseFilesHelper.hpp"
#include "_details/ReferencesManager.hpp"

#include <cstdint>
#include <expected>

namespace CppGit {

/// @brief Result of a rebase operation
enum class RebaseResult : uint8_t
{
    CONFLICT,          ///< Conflict during rebase
    BREAK,             ///<  Stop at break during rebase
    REWORD,            ///< Stop at reword during rebase
    SQUASH,            ///<  Stop at squash during rebase
    EDIT,              ///< Stopped at edit during rebase

    COMMAND_PROCESSED, ///< Command processed successfully (used only internally)
    UNKNOWN_COMMAND,   ///< Unknown command during rebase (used only internally)
    EMPTY_DIFF,        ///< Empty diff during rebase (used only internally)
};

/// @brief Provides functionality to rebase the current branch
class Rebaser
{

public:
    /// @param repo The repository to work with
    explicit Rebaser(const Repository& repository);

    /// @brief Rebase current branch onto upstream branch
    /// @param upstream Upstream branch name to rebase onto
    /// @return Rebased head commit hash if rebase is successful, otherwise Rebase Result error code
    auto rebase(const std::string_view upstream) const -> std::expected<std::string, RebaseResult>;

    /// @brief Rebase current branch onto upstream branch with custom rebase commands
    /// @param upstream Upstream branch name to rebase onto
    /// @param rebaseCommands Custom rebase commands
    /// @return Rebased head commit hash if rebase is successful, otherwise Rebase Result error code
    auto interactiveRebase(const std::string_view upstream, const std::vector<RebaseTodoCommand>& rebaseCommands) const -> std::expected<std::string, RebaseResult>;

    /// @brief Continue stopped rebase
    /// @return Rebased head commit hash if rebase is successful, otherwise Rebase Result error code
    auto continueRebase() const -> std::expected<std::string, RebaseResult>;

    /// @brief Continue stopped rebase and set new commit message if there is a commit that can have its message changed
    ///     May change commit message if: reword, edit, squash or merge conflict
    /// @param message New commit message
    /// @param description New commit description (optional)
    /// @return Rebased head commit hash if rebase is successful, otherwise Rebase Result error code
    auto continueRebase(const std::string_view message, const std::string_view description = "") const -> std::expected<std::string, RebaseResult>;

    /// @brief Abort rebase in progress
    auto abortRebase() const -> void;

    /// @brief Check whether there is a rebase in progress
    /// @return True if there is a rebase in progress, otherwise false
    [[nodiscard]] auto isRebaseInProgress() const -> bool;

    /// @brief Get default rebase commands for rebase onto upstream branch
    ///     All commands are pick commmands by default
    /// @param upstream Upstream branch name that will be rebased onto
    /// @return List of default rebase commands
    [[nodiscard]] auto getDefaultTodoCommands(const std::string_view upstream) const -> std::vector<RebaseTodoCommand>;

    /// @brief Get stopped commit message
    /// @return Stopped commit message
    [[nodiscard]] auto getStoppedMessage() const -> std::string;

private:
    auto rebaseImpl(const std::string_view upstream, const std::vector<RebaseTodoCommand>& rebaseCommands) const -> std::expected<std::string, RebaseResult>;
    auto startRebase(const std::string_view upstream, const std::vector<RebaseTodoCommand>& rebaseCommands) const -> void;
    auto endRebase() const -> std::string;

    auto processTodoList() const -> RebaseResult;
    auto processTodoCommand(const RebaseTodoCommand& rebaseTodoCommand) const -> RebaseResult;
    auto processPickCommand(const RebaseTodoCommand& rebaseTodoCommand) const -> RebaseResult;
    static auto processBreakCommand(const RebaseTodoCommand& /*unused*/) -> RebaseResult;
    auto processReword(const RebaseTodoCommand& rebaseTodoCommand) const -> RebaseResult;
    auto processEdit(const RebaseTodoCommand& rebaseTodoCommand) const -> RebaseResult;
    static auto processDrop(const RebaseTodoCommand& rebaseTodoCommand) -> RebaseResult;
    auto processFixup(const RebaseTodoCommand& rebaseTodoCommand) const -> RebaseResult;
    auto processSquash(const RebaseTodoCommand& rebaseTodoCommand) const -> RebaseResult;

    auto pickCommit(const Commit& commitInfo) const -> std::expected<std::string, RebaseResult>;

    auto isNextCommandFixupOrSquash() const -> bool;

    auto getConcatenatedMessagePreviousAndCurrentCommit(const std::string_view previousCommitHash, const std::string_view currentCommitHash) const -> std::string;

    const Repository* repository;

    CommitsManager commitsManager;
    BranchesManager branchesManager;
    _details::ReferencesManager referencesManager;
    _details::IndexWorktreeManager indexWorktreeManager;
    _details::RebaseFilesHelper rebaseFilesHelper;
    _details::DiffApplier diffApplier;
    _details::CommitAmender commitAmender;
    _details::CommitCreator commitCreator;
};

} // namespace CppGit

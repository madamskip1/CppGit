#pragma once

#include "Commits.hpp"
#include "Error.hpp"
#include "RebaseTodoCommand.hpp"
#include "Repository.hpp"
#include "_details/ApplyDiff.hpp"
#include "_details/IndexWorktree.hpp"
#include "_details/RebaseFilesHelper.hpp"
#include "_details/Refs.hpp"

#include <expected>

namespace CppGit {

class Rebase
{

public:
    /// @param repo The repository to work with
    explicit Rebase(const Repository& repo);

    /// @brief Rebase current branch onto upstream branch
    /// @param upstream Upstream branch name to rebase onto
    /// @return Rebased head commit hash if rebase is successful, otherwise error code
    [[nodiscard]] auto rebase(const std::string_view upstream) const -> std::expected<std::string, Error>;

    /// @brief Rebase current branch onto upstream branch with custom rebase commands
    /// @param upstream Upstream branch name to rebase onto
    /// @param rebaseCommands Custom rebase commands
    /// @return Rebased head commit hash if rebase is successful, otherwise error code
    [[nodiscard]] auto interactiveRebase(const std::string_view upstream, const std::vector<RebaseTodoCommand>& rebaseCommands) const -> std::expected<std::string, Error>;

    /// @brief Continue stopped rebase
    /// @return Rebased head commit hash if rebase is successful, otherwise error code
    [[nodiscard]] auto continueRebase() const -> std::expected<std::string, Error>;

    /// @brief Continue stopped rebase and set new commit message if there is a commit that can have its message changed
    ///     May change commit message if: reword, edit, squash or merge conflict
    /// @param message New commit message
    /// @param description New commit description (optional)
    /// @return Rebased head commit hash if rebase is successful, otherwise error code
    [[nodiscard]] auto continueRebase(const std::string_view message, const std::string_view description = "") const -> std::expected<std::string, Error>;

    /// @brief Abort rebase in progress
    /// @return Error code if there is no rebase in progress, otherwise no error
    auto abortRebase() const -> Error;

    /// @brief Check whether there is a rebase in progress
    /// @return True if there is a rebase in progress, otherwise false
    auto isRebaseInProgress() const -> bool;

    /// @brief Get default rebase commands for rebase onto upstream branch
    ///     All commands are pick commmands by default
    /// @param upstream Upstream branch name that will be rebased onto
    /// @return List of default rebase commands
    auto getDefaultTodoCommands(const std::string_view upstream) const -> std::vector<RebaseTodoCommand>;

    /// @brief Get stopped commit message
    /// @return Stopped commit message
    auto getStoppedMessage() const -> std::string;

private:
    auto rebaseImpl(const std::string_view upstream, const std::vector<RebaseTodoCommand>& rebaseCommands) const -> std::expected<std::string, Error>;
    auto startRebase(const std::string_view upstream, const std::vector<RebaseTodoCommand>& rebaseCommands) const -> void;
    auto endRebase() const -> std::string;

    auto processTodoList() const -> Error;
    auto processTodoCommand(const RebaseTodoCommand& rebaseTodoCommand) const -> Error;
    auto processPickCommand(const RebaseTodoCommand& rebaseTodoCommand) const -> Error;
    static auto processBreakCommand(const RebaseTodoCommand& /*unused*/) -> Error;
    auto processReword(const RebaseTodoCommand& rebaseTodoCommand) const -> Error;
    auto processEdit(const RebaseTodoCommand& rebaseTodoCommand) const -> Error;
    static auto processDrop(const RebaseTodoCommand& rebaseTodoCommand) -> Error;
    auto processFixup(const RebaseTodoCommand& rebaseTodoCommand) const -> Error;
    auto processSquash(const RebaseTodoCommand& rebaseTodoCommand) const -> Error;

    auto pickCommit(const Commit& commitInfo) const -> std::expected<std::string, Error>;

    auto isNextCommandFixupOrSquash() const -> bool;

    auto getConcatenatedMessagePreviousAndCurrentCommit(const std::string_view previousCommitHash, const std::string_view currentCommitHash) const -> std::string;

    const Repository& repo;
    const Commits commits;
    const _details::Refs refs;
    const _details::IndexWorktree indexWorktree;
    const _details::RebaseFilesHelper rebaseFilesHelper;
    const _details::ApplyDiff applyDiff;
};

} // namespace CppGit

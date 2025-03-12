#pragma once

#include "../RebaseTodoCommand.hpp"
#include "../Repository.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace CppGit::_details {

/// @brief Provides internal helper methods to work with rebase files
class RebaseFilesHelper
{
public:
    /// @param repo The repository to work with
    explicit RebaseFilesHelper(const Repository& repo);

    /// @brief Create rebase directory
    ///     .git/rebase-merge
    auto createRebaseDir() const -> void;

    /// @brief Delete all rebase-associated files
    auto deleteAllRebaseFiles() const -> void;

    /// @brief Create onto file
    /// @param onto Commit hash of the branch or commit onto which the current branch is being rebased
    auto createOntoFile(const std::string_view onto) const -> void;

    /// @brief Create head name file
    /// @param branchName Branch name that was checked out before the rebase started
    auto createHeadNameFile(const std::string_view branchName) const -> void;

    /// @brief Get head name file content
    ///     Branch name that was checked out before the rebase started
    [[nodiscard]] auto getHeadName() const -> std::string;

    /// @brief Create orig head files
    /// @param origHead Commit hash of the branch that was checked out before the rebase started
    auto createRebaseOrigHeadFile(const std::string_view origHead) const -> void;

    /// @brief Get orig head file content
    /// @return Commit hash of the branch that was checked out before the rebase started
    [[nodiscard]] auto getOrigHead() const -> std::string;

    /// @brief Create author script file
    /// @param authorName Author name
    /// @param authorEmail Author email
    /// @param authorDate Author date
    auto createAuthorScriptFile(const std::string_view authorName, const std::string_view authorEmail, const std::string_view authorDate) const -> void;

    /// @brief Get author script file content
    /// @return Author name, email and date
    [[nodiscard]] auto getAuthorScriptFile() const -> std::vector<std::string>;

    /// @brief Remove author script file
    auto removeAuthorScriptFile() const -> void;

    /// @brief Create amend file
    /// @param hash Commit hash that will be amended
    auto createAmendFile(const std::string_view hash) const -> void;

    /// @brief Get amend file content
    /// @return Commit hash that will be amended
    [[nodiscard]] auto getAmendFile() const -> std::string;

    /// @brief Remove amend file
    auto removeAmendFile() const -> void;

    /// @brief Create rebase head file
    /// @param hash Commit hash that will be rebased
    auto createRebaseHeadFile(const std::string_view hash) const -> void;

    /// @brief Get rebase head file content
    /// @return Commit hash that will be rebased
    [[nodiscard]] auto getRebaseHeadFile() const -> std::string;

    /// @brief Remove rebase head file
    auto removeRebaseHeadFile() const -> void;

    /// @brief Append rewritten list file
    /// @param hashBefore Commit hash before rewriting
    /// @param hashAfter Commit hash after rewriting
    auto appendRewrittenListFile(const std::string_view hashBefore, const std::string_view hashAfter) const -> void;

    /// @brief Append rewritten pending file
    /// @param hash Commit hash that is pending to be rewritten
    auto appendRewrittenPendingFile(const std::string_view hash) const -> void;

    /// @brief Append rewritten list with rewritten pending file
    /// @param newHash Commit hash after rewriting
    auto appendRewrittenListWithRewrittenPending(const std::string_view newHash) const -> void;

    /// @brief Append current fixup file
    /// @param rebaseTodoCommand Fixup command
    auto appendCurrentFixupFile(const RebaseTodoCommand& rebaseTodoCommand) const -> void;

    /// @brief Check whether there are any squash in current fixup
    /// @return True if there are any squash, false otherwise
    [[nodiscard]] auto areAnySquashInCurrentFixup() const -> bool;

    /// @brief Remove current fixup file
    auto removeCurrentFixupFile() const -> void;

    /// @brief Create message file
    /// @param message Commit message
    auto createMessageFile(const std::string_view message) const -> void;

    /// @brief Get message file content
    /// @return Commit message
    [[nodiscard]] auto getMessageFile() const -> std::string;

    /// @brief Remove message file
    auto removeMessageFile() const -> void;

    /// @brief Generate todo file
    /// @param rebaseTodoCommands List of rebase todo commands
    auto generateTodoFile(const std::vector<RebaseTodoCommand>& rebaseTodoCommands) const -> void;

    /// @brief Peek todo file
    /// @return Next rebase todo command
    [[nodiscard]] auto peekTodoFile() const -> std::optional<RebaseTodoCommand>;

    /// @brief Pop todo file
    ///     Remove first line from todo file
    auto popTodoFile() const -> void;

    /// @brief Peek and pop todo file
    ///     Peek first line from todo file and remove it
    /// @return Next rebase todo command
    [[nodiscard]] auto peakAndPopTodoFile() const -> std::optional<RebaseTodoCommand>;

    /// @brief Append done file
    /// @param rebaseTodoCommand Done rebase todo command
    auto appendDoneFile(const RebaseTodoCommand& rebaseTodoCommand) const -> void;

    /// @brief Get last done command
    /// @return Last done rebase todo command
    [[nodiscard]] auto getLastDoneCommand() const -> std::optional<RebaseTodoCommand>;

private:
    const Repository* repo;

    static auto parseTodoCommandLine(const std::string_view line) -> std::optional<RebaseTodoCommand>;
};

} // namespace CppGit::_details

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
    explicit Rebase(const Repository& repo);

    [[nodiscard]] auto rebase(const std::string_view upstream) const -> std::expected<std::string, Error>;
    [[nodiscard]] auto interactiveRebase(const std::string_view upstream, const std::vector<RebaseTodoCommand>& rebaseCommands) const -> std::expected<std::string, Error>;

    [[nodiscard]] auto continueRebase() const -> std::expected<std::string, Error>;
    [[nodiscard]] auto continueRebase(const std::string_view message, const std::string_view description = "") const -> std::expected<std::string, Error>;

    auto abortRebase() const -> Error;
    auto isRebaseInProgress() const -> bool;

    auto getDefaultTodoCommands(const std::string_view upstream) const -> std::vector<RebaseTodoCommand>;
    auto getStoppedMessage() const -> std::string;

private:
    auto rebaseImpl(const std::string_view upstream, const std::vector<RebaseTodoCommand>& rebaseCommands) const -> std::expected<std::string, Error>;
    auto startRebase(const std::string_view upstream, const std::vector<RebaseTodoCommand>& rebaseCommands) const -> void;
    auto endRebase() const -> std::string;

    auto processTodoList() const -> Error;
    auto processTodoCommand(const RebaseTodoCommand& rebaseTodoCommand) const -> Error;
    auto processPickCommand(const RebaseTodoCommand& rebaseTodoCommand) const -> Error;
    static auto processBreakCommand(const RebaseTodoCommand&) -> Error;
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

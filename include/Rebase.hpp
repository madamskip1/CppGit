#pragma once

#include "CherryPick.hpp"
#include "Commit.hpp"
#include "Error.hpp"
#include "Index.hpp"
#include "RebaseTodoCommand.hpp"
#include "Repository.hpp"
#include "_details/ApplyDiff.hpp"
#include "_details/IndexWorktree.hpp"
#include "_details/RebaseFilesHelper.hpp"
#include "_details/Refs.hpp"

#include <expected>
#include <optional>

namespace CppGit {

class Rebase
{

public:
    explicit Rebase(const Repository& repo);

    auto rebase(const std::string_view upstream) const -> std::expected<std::string, Error>;
    auto interactiveRebase(const std::string_view upstream, const std::vector<RebaseTodoCommand>& rebaseCommands) const -> std::expected<std::string, Error>;

    auto continueRebase() const -> std::expected<std::string, Error>;
    auto continueRebase(const std::string_view message, const std::string_view description = "") const -> std::expected<std::string, Error>;

    auto abortRebase() const -> Error;
    auto isRebaseInProgress() const -> bool;

    auto getDefaultTodoCommands(const std::string_view upstream) const -> std::vector<RebaseTodoCommand>;
    auto getSquashMessage() const -> std::string;

private:
    auto rebaseImpl(const std::string_view upstream, const std::vector<RebaseTodoCommand>& rebaseCommands) const -> std::expected<std::string, Error>;
    auto startRebase(const std::string_view upstream, const std::vector<RebaseTodoCommand>& rebaseCommands) const -> void;
    auto endRebase() const -> std::string;

    auto processTodoList() const -> Error;
    auto processTodoCommand(const RebaseTodoCommand& rebaseTodoCommand) const -> Error;
    auto processPickCommand(const RebaseTodoCommand& rebaseTodoCommand) const -> Error;
    auto processBreakCommand(const RebaseTodoCommand&) const -> Error;
    auto processReword(const RebaseTodoCommand& rebaseTodoCommand) const -> Error;
    auto processEdit(const RebaseTodoCommand& rebaseTodoCommand) const -> Error;
    auto processDrop(const RebaseTodoCommand& rebaseTodoCommand) const -> Error;
    auto processFixup(const RebaseTodoCommand& rebaseTodoCommand) const -> Error;
    auto processSquash(const RebaseTodoCommand& rebaseTodoCommand) const -> Error;

    auto pickCommit(const RebaseTodoCommand& rebaseTodoCommand) const -> std::expected<std::string, Error>;

    auto startConflict(const RebaseTodoCommand& rebaseTodoCommand) const -> void;

    auto isNextCommandFixupOrSquash() const -> bool;

    auto continueEditImpl() const -> std::expected<std::string, Error>;
    auto continueRewordImpl(const RebaseTodoCommand& lastDoneCommand, const std::string_view message, const std::string_view description) const -> std::expected<std::string, Error>;
    auto continueSquashImpl(const std::string_view message, const std::string_view description) const -> std::expected<std::string, Error>;
    auto continueConflictImpl(const std::string_view stoppedSha) const -> std::expected<std::string, Error>;

    auto concatMessageAndDescription(const std::string_view message, const std::string_view description) const -> std::string;
    auto getConcatenatedMessagePreviousAndCurrentCommit(const std::string_view previousCommitHash, const std::string_view currentCommitHash) const -> std::string;

    const Repository& repo;
    const _details::Refs refs;
    const _details::IndexWorktree indexWorktree;
    const CherryPick cherryPick;
    const _details::RebaseFilesHelper rebaseFilesHelper;
    const _details::ApplyDiff applyDiff;
    const Index index;
};

} // namespace CppGit

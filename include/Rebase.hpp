#pragma once

#include "CherryPick.hpp"
#include "Commit.hpp"
#include "Error.hpp"
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
    auto continueReword(const std::string_view message = "", const std::string_view description = "") const -> std::expected<std::string, Error>;
    auto abortRebase() const -> Error;
    auto isRebaseInProgress() const -> bool;

    auto getDefaultTodoCommands(const std::string_view upstream) const -> std::vector<RebaseTodoCommand>;

private:
    auto rebaseImpl(const std::string_view upstream, const std::vector<RebaseTodoCommand>& rebaseCommands) const -> std::expected<std::string, Error>;
    auto startRebase(const std::string_view upstream, const std::vector<RebaseTodoCommand>& rebaseCommands) const -> void;
    auto endRebase() const -> std::string;

    auto processTodoList() const -> Error;
    auto processTodoCommand(const RebaseTodoCommand& rebaseTodoCommand) const -> Error;
    auto processPickCommand(const RebaseTodoCommand& rebaseTodoCommand) const -> Error;
    auto processBreakCommand(const RebaseTodoCommand&) const -> Error;
    auto processReword(const RebaseTodoCommand& rebaseTodoCommand) const -> Error;

    auto startConflict(const RebaseTodoCommand& rebaseTodoCommand) const -> void;

    const Repository& repo;
    const _details::Refs refs;
    const _details::IndexWorktree indexWorktree;
    const CherryPick cherryPick;
    const _details::RebaseFilesHelper rebaseFilesHelper;
    const _details::ApplyDiff applyDiff;
};

} // namespace CppGit

#pragma once

#include "CherryPick.hpp"
#include "Commit.hpp"
#include "Error.hpp"
#include "RebaseTodoCommand.hpp"
#include "Repository.hpp"
#include "_details/IndexWorktree.hpp"
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
    auto abortRebase() const -> Error;
    auto isRebaseInProgress() const -> bool;

    auto getDefaultTodoCommands(const std::string_view upstream) const -> std::vector<RebaseTodoCommand>;

private:
    auto rebaseImpl(const std::string_view upstream, const std::vector<RebaseTodoCommand>& rebaseCommands) const -> std::expected<std::string, Error>;
    auto startRebase(const std::string_view upstream, const std::vector<RebaseTodoCommand>& rebaseCommands) const -> void;
    auto endRebase() const -> std::string;

    auto createRebaseDir() const -> void;
    auto deleteAllRebaseFiles() const -> void;
    auto createHeadNameFile(const std::string_view branchName) const -> void;
    auto createOntoFile(const std::string_view onto) const -> void;
    auto createOrigHeadFiles(const std::string_view origHead) const -> void;
    auto createStoppedShaFile(const std::string_view hash) const -> void;
    auto getHeadName() const -> std::string;
    auto getOrigHead() const -> std::string;
    auto getStoppedShaFile() const -> std::string;

    auto generateTodoFile(const std::vector<RebaseTodoCommand>& rebaseCommands) const -> void;
    auto peekTodoCommand() const -> std::optional<RebaseTodoCommand>;

    auto processTodoList() const -> Error;
    auto processTodoCommand(const RebaseTodoCommand& rebaseTodoCommand) const -> Error;
    auto processPickCommand(const RebaseTodoCommand& rebaseTodoCommand) const -> Error;
    auto processBreakCommand(const RebaseTodoCommand&) const -> Error;
    auto appendTodoCommandToDoneList(const RebaseTodoCommand& rebaseTodoCommand) const -> void;
    auto popTodoCommandFromTodoList() const -> void;
    static auto parseTodoCommandLine(const std::string_view line) -> std::optional<RebaseTodoCommand>;

    auto startConflict(const RebaseTodoCommand& rebaseTodoCommand) const -> void;

    const Repository& repo;
    const _details::Refs refs;
    const _details::IndexWorktree indexWorktree;
    const CherryPick cherryPick;
};

} // namespace CppGit

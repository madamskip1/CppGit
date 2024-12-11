#pragma once

#include "CherryPick.hpp"
#include "Commit.hpp"
#include "Error.hpp"
#include "Repository.hpp"
#include "_details/IndexWorktree.hpp"
#include "_details/Refs.hpp"

#include <expected>

namespace CppGit {

class Rebase
{

public:
    explicit Rebase(const Repository& repo);

    auto rebase(const std::string_view upstream) const -> std::expected<std::string, Error>;
    auto continueRebase() const -> std::expected<std::string, Error>;
    auto abortRebase() const -> Error;
    auto isRebaseInProgress() const -> bool;

private:
    struct TodoLine
    {
        std::string command;
        std::string commitHash;
        std::string message;
    };

    auto startRebase(const std::string_view upstream) const -> void;
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

    auto generateTodoFile(const std::vector<Commit>& commits) const -> void;
    auto nextTodo() const -> TodoLine;

    auto processTodoList() const -> Error;
    auto processTodo(const TodoLine& todoLine) const -> Error;
    auto processPick(const TodoLine& todoLine) const -> Error;
    auto todoDone(const TodoLine& todoLine) const -> void;
    static auto parseTodoLine(const std::string_view line) -> TodoLine;

    auto startConflict(const TodoLine& todoLine) const -> void;

    const Repository& repo;
    const _details::Refs refs;
    const _details::IndexWorktree indexWorktree;
    const CherryPick cherryPick;
};

} // namespace CppGit

#pragma once

#include "CherryPick.hpp"
#include "Commit.hpp"
#include "Repository.hpp"
#include "_details/IndexWorktree.hpp"
#include "_details/Refs.hpp"


namespace CppGit {

class Rebase
{

public:
    explicit Rebase(const Repository& repo);

    auto rebase(const std::string_view upstream) const -> void;
    auto abortRebase() const -> void;
    auto continueRebase() const -> void;

private:
    struct TodoLine
    {
        std::string command;
        std::string commitHash;
        std::string message;
    };

    auto startRebase(const std::string_view upstream) const -> void;
    auto endRebase() const -> void;

    auto createRebaseDir() const -> void;
    auto deleteAllRebaseFiles() const -> void;
    auto createHeadNameFile(const std::string_view branchName) const -> void;
    auto getHeadName() const -> std::string;
    auto createOntoFile(const std::string_view onto) const -> void;
    auto createOrigHeadFiles(const std::string_view origHead) const -> void;
    auto getOrigHead() const -> std::string;
    auto createStoppedShaFile(const std::string_view hash) const -> void;
    auto getStoppedShaFile() const -> std::string;

    auto generateTodoFile(const std::vector<Commit>& commits) const -> void;
    auto nextTodo() const -> TodoLine;

    auto processTodoList() const -> void;
    auto processTodo(const TodoLine& todoLine) const -> void;
    auto processPick(const TodoLine& todoLine) const -> void;
    auto todoDone(const TodoLine& todoLine) const -> void;
    auto startConflict(const TodoLine& todoLine) const -> void;

    static auto parseTodoLine(const std::string_view line) -> TodoLine;

    const Repository& repo;
    const _details::Refs refs;
    const _details::IndexWorktree indexWorktree;
    const CherryPick cherryPick;
};

} // namespace CppGit

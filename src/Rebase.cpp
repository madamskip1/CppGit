#include "Rebase.hpp"

#include "Branches.hpp"
#include "CherryPick.hpp"
#include "Commit.hpp"
#include "CommitsHistory.hpp"
#include "Exceptions.hpp"

#include <filesystem>
#include <fstream>
#include <vector>

namespace CppGit {
Rebase::Rebase(const Repository& repo)
    : repo(repo),
      refs(_details::Refs(repo)),
      cherryPick(CherryPick(repo))
{
}
auto Rebase::rebase(const std::string_view upstream) const -> void
{
    startRebase(upstream);
    processTodoList();
    endRebase();
}

auto Rebase::abort() const -> void
{
}

auto Rebase::startRebase(const std::string_view upstream) const -> void
{
    auto mergeBase = repo.executeGitCommand("merge-base", "HEAD", upstream);

    if (mergeBase.return_code != 0)
    {
        throw std::runtime_error("Failed to find merge base");
    }

    const auto& mergeBaseSha = mergeBase.stdout;
    auto branches = repo.Branches();
    auto commitsHistory = repo.CommitsHistory();
    commitsHistory.setOrder(CommitsHistory::Order::REVERSE);
    auto commitsToRebase = commitsHistory.getCommitsLogDetailed(mergeBaseSha, "HEAD");
    auto upstreamHash = refs.getRefHash(upstream);

    createRebaseDir();
    createHeadNameFile(branches.getCurrentBranch());
    createOntoFile(upstreamHash);
    createOrigHeadFiles(refs.getRefHash("HEAD"));
    generateTodoFile(commitsToRebase);

    branches.detachHead(upstreamHash);
}

auto Rebase::endRebase() const -> void
{
    auto currentHash = refs.getRefHash("HEAD");
    auto headName = getHeadName();
    refs.updateRefHash(headName, currentHash);
    refs.updateSymbolicRef("HEAD", headName);
}

auto Rebase::createRebaseDir() const -> void
{
    std::filesystem::create_directory(repo.getGitDirectoryPath() / "rebase-merge");
}

auto Rebase::deleteAllRebaseFiles() const -> void
{
    std::filesystem::remove_all(repo.getGitDirectoryPath() / "rebase-merge");
    std::filesystem::remove(repo.getGitDirectoryPath() / "ORIG_HEAD");
    std::filesystem::remove(repo.getGitDirectoryPath() / "REBASE_HEAD");
}

auto Rebase::createHeadNameFile(const std::string_view branchName) const -> void
{
    // Indicates the branch that was checked out before the rebase started
    auto file = std::ofstream(repo.getGitDirectoryPath() / "rebase-merge" / "head-name");
    file << branchName;
    file.close();
}

auto Rebase::getHeadName() const -> std::string
{
    auto file = std::ifstream(repo.getGitDirectoryPath() / "rebase-merge" / "head-name");
    std::string headName;
    file >> headName;
    file.close();
    return headName;
}

auto Rebase::createOntoFile(const std::string_view onto) const -> void
{
    // Contains the commit hash of the branch or commit onto which the current branch is being rebased
    auto file = std::ofstream(repo.getGitDirectoryPath() / "rebase-merge" / "onto");
    file << onto;
    file.close();
}

auto Rebase::createOrigHeadFiles(const std::string_view origHead) const -> void
{
    // Contains the commit hash of the branch that was checked out before the rebase started
    auto file = std::ofstream(repo.getGitDirectoryPath() / "rebase-merge" / "orig-head");
    file << origHead;
    file.close();

    file = std::ofstream(repo.getGitDirectoryPath() / "ORIG_HEAD");
    file << origHead;
    file.close();
}

auto Rebase::getOrigHead() const -> std::string
{
    auto file = std::ifstream(repo.getGitDirectoryPath() / "rebase-merge" / "orig-head");
    std::string origHead;
    file >> origHead;
    file.close();
    return origHead;
}

auto Rebase::generateTodoFile(const std::vector<Commit>& commits) const -> void
{
    auto file = std::ofstream{ repo.getGitDirectoryPath() / "rebase-merge" / "git-rebase-todo" };
    for (const auto& commit : commits)
    {
        file << "pick " << commit.getHash() << " " << commit.getMessage() << "\n";
    }
    file.close();

    std::filesystem::copy(repo.getGitDirectoryPath() / "rebase-merge" / "git-rebase-todo", repo.getGitDirectoryPath() / "rebase-merge" / "git-rebase-todo.backup");
}

auto Rebase::nextTodo() const -> TodoLine
{
    auto todoFilePath = repo.getGitDirectoryPath() / "rebase-merge" / "git-rebase-todo";
    auto tempFilePath = repo.getGitDirectoryPath() / "rebase-merge" / "git-rebase-todo.temp";

    auto todoFile = std::ifstream{ todoFilePath };
    auto tempFile = std::ofstream{ tempFilePath };

    std::string todoLine;
    std::string line;
    bool firstLine = true;

    while (std::getline(todoFile, line))
    {
        if (firstLine)
        {
            todoLine = line;
            firstLine = false;
            continue;
        }
        tempFile << line << "\n";
    }

    todoFile.close();
    tempFile.close();

    std::filesystem::remove(todoFilePath);
    std::filesystem::rename(tempFilePath, todoFilePath);

    auto todo = parseTodoLine(todoLine);
    auto messageFile = std::ofstream{ repo.getGitDirectoryPath() / "rebase-merge" / "message" };
    messageFile << todo.message;
    messageFile.close();

    return todo;
}

auto Rebase::processTodoList() const -> void
{
    auto todo = nextTodo();
    while (!todo.command.empty())
    {
        try
        {
            processTodo(todo);
        }
        catch (MergeConflict& e)
        {
            startConflict(todo);
        }

        todoDone(todo);
        todo = nextTodo();
    }
}

auto Rebase::processTodo(const TodoLine& todoLine) const -> void
{
    if (todoLine.command == "pick")
    {
        processPick(todoLine);
    }
    else
    {
        throw std::runtime_error("Todo command not yet implemented");
    }
}

auto Rebase::processPick(const TodoLine& todoLine) const -> void
{
    cherryPick.cherryPickCommit(todoLine.commitHash, CherryPickEmptyCommitStrategy::KEEP);
}

auto Rebase::todoDone(const TodoLine& todoLine) const -> void
{
    auto doneFile = std::ofstream{ repo.getGitDirectoryPath() / "rebase-merge" / "done", std::ios::app };
    doneFile << todoLine.command << " " << todoLine.commitHash << " " << todoLine.message << "\n";
    doneFile.close();
}

auto Rebase::startConflict(const TodoLine& todoLine) const -> void
{
    auto conflictFile = std::ofstream{ repo.getGitDirectoryPath() / "rebase-merge" / "stopped-sha" };
    conflictFile << todoLine.commitHash;
    conflictFile.close();

    todoDone(todoLine);

    throw CppGit::MergeConflict{};
}

auto Rebase::parseTodoLine(const std::string_view line) -> TodoLine
{
    std::size_t startPos = 0;
    std::size_t endPos = 0;

    endPos = line.find(' ', startPos);
    auto command = std::string{ line.substr(startPos, endPos - startPos) };

    startPos = endPos + 1;
    endPos = line.find(' ', startPos);
    auto commitHash = std::string{ line.substr(startPos, endPos - startPos) };

    auto message = std::string{ line.substr(endPos + 1) };

    return TodoLine{ command, commitHash, message };
}

} // namespace CppGit

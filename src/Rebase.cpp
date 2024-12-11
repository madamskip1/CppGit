#include "Rebase.hpp"

#include "Branches.hpp"
#include "CherryPick.hpp"
#include "Commit.hpp"
#include "Commits.hpp"
#include "CommitsHistory.hpp"
#include "_details/CreateCommit.hpp"
#include "_details/FileUtility.hpp"

#include <filesystem>
#include <fstream>
#include <vector>

namespace CppGit {
Rebase::Rebase(const Repository& repo)
    : repo(repo),
      refs(repo),
      cherryPick(repo),
      indexWorktree(repo)
{
}
auto Rebase::rebase(const std::string_view upstream) const -> std::expected<std::string, Error>
{
    startRebase(upstream);

    auto todoResult = processTodoList();
    if (todoResult != Error::NONE)
    {
        return std::unexpected{ todoResult };
    }

    return endRebase();
}

auto Rebase::abortRebase() const -> Error
{
    if (!isRebaseInProgress())
    {
        return Error::NO_REBASE_IN_PROGRESS;
    }

    indexWorktree.resetIndexToTree(getOrigHead());
    indexWorktree.copyForceIndexToWorktree();
    refs.updateSymbolicRef("HEAD", getHeadName());

    deleteAllRebaseFiles();

    return Error::NONE;
}

auto Rebase::continueRebase() const -> std::expected<std::string, Error>
{
    if (!isRebaseInProgress())
    {
        return std::unexpected{ Error::NO_REBASE_IN_PROGRESS };
    }

    auto commitHash = getStoppedShaFile();
    auto commits = Commits{ repo };
    auto _createCommit = _details::CreateCommit{ repo };
    auto commitInfo = commits.getCommitInfo(commitHash);

    auto envp = std::vector<std::string>{
        "GIT_AUTHOR_NAME=" + commitInfo.getAuthor().name,
        "GIT_AUTHOR_EMAIL=" + commitInfo.getAuthor().email,
        "GIT_AUTHOR_DATE=" + commitInfo.getAuthorDate()
    };

    auto parent = commits.hasAnyCommits() ? commits.getHeadCommitHash() : std::string{};

    _createCommit.createCommit(commitInfo.getMessage(), commitInfo.getDescription(), { parent }, envp);

    processTodoList();

    return endRebase();
}

auto Rebase::isRebaseInProgress() const -> bool
{
    if (std::ifstream headFile(repo.getGitDirectoryPath() / "REBASE_HEAD"); headFile.is_open())
    {
        auto isEmpty = headFile.peek() == std::ifstream::traits_type::eof();
        headFile.close();

        return !isEmpty;
    }

    return false;
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

auto Rebase::endRebase() const -> std::string
{
    auto currentHash = refs.getRefHash("HEAD");
    auto headName = getHeadName();
    refs.updateRefHash(headName, currentHash);
    refs.updateSymbolicRef("HEAD", headName);

    deleteAllRebaseFiles();

    return currentHash;
}

auto Rebase::createRebaseDir() const -> void
{
    std::filesystem::create_directory(repo.getGitDirectoryPath() / "rebase-merge");
}

auto Rebase::deleteAllRebaseFiles() const -> void
{
    std::filesystem::remove_all(repo.getGitDirectoryPath() / "rebase-merge");
    std::filesystem::remove(repo.getGitDirectoryPath() / "REBASE_HEAD");
}

auto Rebase::createHeadNameFile(const std::string_view branchName) const -> void
{
    // Indicates the branch that was checked out before the rebase started
    _details::FileUtility::createOrOverwriteFile(repo.getGitDirectoryPath() / "rebase-merge/head-name", branchName);
}

auto Rebase::getHeadName() const -> std::string
{
    return _details::FileUtility::readFile(repo.getGitDirectoryPath() / "rebase-merge" / "head-name");
}

auto Rebase::createOntoFile(const std::string_view onto) const -> void
{
    // Contains the commit hash of the branch or commit onto which the current branch is being rebased

    _details::FileUtility::createOrOverwriteFile(repo.getGitDirectoryPath() / "rebase-merge/onto", onto);
}

auto Rebase::createOrigHeadFiles(const std::string_view origHead) const -> void
{
    // Contains the commit hash of the branch that was checked out before the rebase started
    _details::FileUtility::createOrOverwriteFile(repo.getGitDirectoryPath() / "rebase-merge/orig-head", origHead);
    _details::FileUtility::createOrOverwriteFile(repo.getGitDirectoryPath() / "ORIG_HEAD", origHead);
}

auto Rebase::getOrigHead() const -> std::string
{
    return _details::FileUtility::readFile(repo.getGitDirectoryPath() / "rebase-merge/orig-head");
}

auto Rebase::createStoppedShaFile(const std::string_view hash) const -> void
{
    _details::FileUtility::createOrOverwriteFile(repo.getGitDirectoryPath() / "rebase-merge/stopped-sha", hash);
}

auto Rebase::getStoppedShaFile() const -> std::string
{
    return _details::FileUtility::readFile(repo.getGitDirectoryPath() / "rebase-merge/stopped-sha");
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

    _details::FileUtility::createOrOverwriteFile(repo.getGitDirectoryPath() / "rebase-merge/message", todo.message);
    _details::FileUtility::createOrOverwriteFile(repo.getGitDirectoryPath() / "REBASE_HEAD", todo.commitHash);

    return todo;
}

auto Rebase::processTodoList() const -> Error
{
    auto todo = nextTodo();
    while (!todo.command.empty())
    {
        auto todoResult = processTodo(todo);

        if (todoResult == Error::REBASE_CONFLICT)
        {
            startConflict(todo);
            return Error::REBASE_CONFLICT;
        }

        todoDone(todo);
        todo = nextTodo();
    }

    return Error::NONE;
}

auto Rebase::processTodo(const TodoLine& todoLine) const -> Error
{
    if (todoLine.command == "pick")
    {
        return processPick(todoLine);
    }
    else
    {
        throw std::runtime_error("Todo command not yet implemented");
    }
}

auto Rebase::processPick(const TodoLine& todoLine) const -> Error
{
    auto cherryPickResult = cherryPick.cherryPickCommit(todoLine.commitHash, CherryPickEmptyCommitStrategy::KEEP).error_or(Error::NONE);

    if (cherryPickResult == Error::NONE)
    {
        return Error::NONE;
    }
    else if (cherryPickResult == Error::CHERRY_PICK_CONFLICT)
    {
        return Error::REBASE_CONFLICT;
    }
    else
    {
        throw std::runtime_error("Unexpected error during cherry-pick");
    }
}

auto Rebase::todoDone(const TodoLine& todoLine) const -> void
{
    _details::FileUtility::createOrAppendFile(repo.getGitDirectoryPath() / "rebase-merge/done", todoLine.command, " ", todoLine.commitHash, " ", todoLine.message, "\n");
}

auto Rebase::startConflict(const TodoLine& todoLine) const -> void
{
    createStoppedShaFile(todoLine.commitHash);

    todoDone(todoLine);
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

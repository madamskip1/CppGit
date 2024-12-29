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
      indexWorktree(repo),
      cherryPick(repo)
{
}
auto Rebase::rebase(const std::string_view upstream) const -> std::expected<std::string, Error>
{
    auto rebaseComamnds = getDefaultTodoCommands(upstream);
    return rebaseImpl(upstream, rebaseComamnds);
}

auto Rebase::interactiveRebase(const std::string_view upstream, const std::vector<RebaseTodoCommand>& rebaseCommands) const -> std::expected<std::string, Error>
{
    return rebaseImpl(upstream, rebaseCommands);
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

    return Error::NO_ERROR;
}

auto Rebase::continueRebase() const -> std::expected<std::string, Error>
{
    if (!isRebaseInProgress())
    {
        return std::unexpected{ Error::NO_REBASE_IN_PROGRESS };
    }

    if (auto stoppedHash = getStoppedShaFile(); stoppedHash != "")
    {
        auto commits = Commits{ repo };
        auto _createCommit = _details::CreateCommit{ repo };
        auto commitInfo = commits.getCommitInfo(stoppedHash);

        auto envp = std::vector<std::string>{
            "GIT_AUTHOR_NAME=" + commitInfo.getAuthor().name,
            "GIT_AUTHOR_EMAIL=" + commitInfo.getAuthor().email,
            "GIT_AUTHOR_DATE=" + commitInfo.getAuthorDate()
        };

        auto parent = commits.hasAnyCommits() ? commits.getHeadCommitHash() : std::string{};

        _createCommit.createCommit(commitInfo.getMessage(), commitInfo.getDescription(), { parent }, envp);
    }

    processTodoList();

    return endRebase();
}

auto Rebase::isRebaseInProgress() const -> bool
{
    return std::filesystem::exists(repo.getGitDirectoryPath() / "rebase-merge" / "git-rebase-todo");
}

auto Rebase::getDefaultTodoCommands(const std::string_view upstream) const -> std::vector<RebaseTodoCommand>
{
    auto rebaseBase = repo.executeGitCommand("merge-base", "HEAD", upstream);

    if (rebaseBase.return_code != 0)
    {
        throw std::runtime_error("Failed to find merge base");
    }

    const auto& rebaseBaseSha = rebaseBase.stdout;

    auto commitsHistory = repo.CommitsHistory();
    commitsHistory.setOrder(CommitsHistory::Order::REVERSE);
    auto commitsToRebase = commitsHistory.getCommitsLogDetailed(rebaseBaseSha, "HEAD");

    auto rebaseCommands = std::vector<RebaseTodoCommand>{};

    for (const auto& commit : commitsToRebase)
    {
        auto commitHash = commit.getHash();
        auto commitMessage = commit.getMessage();

        rebaseCommands.emplace_back(RebaseTodoCommandType::PICK, commitHash, commitMessage);
    }

    return rebaseCommands;
}

auto Rebase::rebaseImpl(const std::string_view upstream, const std::vector<RebaseTodoCommand>& rebaseCommands) const -> std::expected<std::string, Error>
{
    startRebase(upstream, rebaseCommands);

    if (auto todoResult = processTodoList(); todoResult != Error::NO_ERROR)
    {
        return std::unexpected{ todoResult };
    }

    return endRebase();
}

auto Rebase::startRebase(const std::string_view upstream, const std::vector<RebaseTodoCommand>& rebaseCommands) const -> void
{
    auto branches = repo.Branches();
    auto upstreamHash = refs.getRefHash(upstream);

    createRebaseDir();
    createHeadNameFile(branches.getCurrentBranch());
    createOntoFile(upstreamHash);
    createOrigHeadFiles(refs.getRefHash("HEAD"));
    generateTodoFile(rebaseCommands);

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

auto Rebase::generateTodoFile(const std::vector<RebaseTodoCommand>& rebaseCommands) const -> void
{
    auto file = std::ofstream{ repo.getGitDirectoryPath() / "rebase-merge" / "git-rebase-todo" };
    for (const auto& command : rebaseCommands)
    {
        file << command.toString() << "\n";
    }
    file.close();

    std::filesystem::copy(repo.getGitDirectoryPath() / "rebase-merge" / "git-rebase-todo", repo.getGitDirectoryPath() / "rebase-merge" / "git-rebase-todo.backup");
}

auto Rebase::peekTodoCommand() const -> std::optional<RebaseTodoCommand>
{
    auto todoFilePath = repo.getGitDirectoryPath() / "rebase-merge" / "git-rebase-todo";
    auto todoFile = std::ifstream{ todoFilePath };
    std::string todoLine;
    std::getline(todoFile, todoLine);
    todoFile.close();

    auto commandTodo = parseTodoCommandLine(todoLine);

    const auto& hash = commandTodo.has_value() ? commandTodo.value().hash : "";
    const auto& message = commandTodo.has_value() ? commandTodo.value().message : "";

    return commandTodo;
}

auto Rebase::processTodoList() const -> Error
{
    auto todoCommand = peekTodoCommand();
    while (todoCommand)
    {
        auto todoResult = processTodoCommand(todoCommand.value());
        appendTodoCommandToDoneList(todoCommand.value());
        popTodoCommandFromTodoList();

        if (todoResult == Error::REBASE_CONFLICT)
        {
            startConflict(todoCommand.value());
            return Error::REBASE_CONFLICT;
        }

        if (todoResult == Error::REBASE_BREAK)
        {
            return Error::REBASE_BREAK;
        }

        todoCommand = peekTodoCommand();
    }

    return Error::NO_ERROR;
}

auto Rebase::processTodoCommand(const RebaseTodoCommand& rebaseTodoCommand) const -> Error
{
    if (rebaseTodoCommand.type == RebaseTodoCommandType::PICK)
    {
        return processPickCommand(rebaseTodoCommand);
    }

    if (rebaseTodoCommand.type == RebaseTodoCommandType::BREAK)
    {
        return processBreakCommand(rebaseTodoCommand);
    }

    throw std::runtime_error("Todo command not yet implemented");
}

auto Rebase::processPickCommand(const RebaseTodoCommand& rebaseTodoCommand) const -> Error
{
    auto commits = Commits{ repo };
    auto headCommitHash = commits.getHeadCommitHash();

    if (auto pickedCommitInfo = commits.getCommitInfo(rebaseTodoCommand.hash); pickedCommitInfo.getParents()[0] == headCommitHash)
    {
        // can FastForward
        indexWorktree.resetIndexToTree(rebaseTodoCommand.hash);
        indexWorktree.copyForceIndexToWorktree();
        refs.detachHead(rebaseTodoCommand.hash);

        return Error::NO_ERROR;
    }

    auto cherryPickResult = cherryPick.cherryPickCommit(rebaseTodoCommand.hash, CherryPickEmptyCommitStrategy::KEEP).error_or(Error::NO_ERROR);

    if (cherryPickResult == Error::NO_ERROR)
    {
        return Error::NO_ERROR;
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

auto Rebase::processBreakCommand(const RebaseTodoCommand&) const -> Error
{
    return Error::REBASE_BREAK;
}

auto Rebase::appendTodoCommandToDoneList(const RebaseTodoCommand& rebaseTodoCommand) const -> void
{
    auto x = rebaseTodoCommand.toString();
    _details::FileUtility::createOrAppendFile(repo.getGitDirectoryPath() / "rebase-merge/done", rebaseTodoCommand.toString(), "\n");
}

auto Rebase::popTodoCommandFromTodoList() const -> void
{
    auto todoFilePath = repo.getGitDirectoryPath() / "rebase-merge" / "git-rebase-todo";
    auto tempFilePath = repo.getGitDirectoryPath() / "rebase-merge" / "git-rebase-todo.temp";

    auto todoFile = std::ifstream{ todoFilePath };
    auto tempFile = std::ofstream{ tempFilePath };

    std::string line;
    std::getline(todoFile, line); // skip first line

    while (std::getline(todoFile, line))
    {
        tempFile << line << "\n";
    }

    todoFile.close();
    tempFile.close();

    std::filesystem::remove(todoFilePath);
    std::filesystem::rename(tempFilePath, todoFilePath);
}

auto Rebase::startConflict(const RebaseTodoCommand& rebaseTodoCommand) const -> void
{
    createStoppedShaFile(rebaseTodoCommand.hash);
}

auto Rebase::parseTodoCommandLine(const std::string_view line) -> std::optional<RebaseTodoCommand>
{
    if (line.empty())
    {
        return std::nullopt;
    }

    std::size_t startPos = 0;
    std::size_t endPos = 0;

    endPos = line.find(' ', startPos);
    auto command = std::string{ line.substr(startPos, endPos - startPos) };

    if (endPos == std::string::npos)
    {
        return RebaseTodoCommand{ RebaseTodoCommandTypeWrapper::fromString(command) };
    }

    startPos = endPos + 1;
    endPos = line.find(' ', startPos);
    auto commitHash = std::string{ line.substr(startPos, endPos - startPos) };

    auto message = std::string{ line.substr(endPos + 1) };

    return RebaseTodoCommand{ RebaseTodoCommandTypeWrapper::fromString(command), commitHash, message };
}

} // namespace CppGit

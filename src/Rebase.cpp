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
      cherryPick(repo),
      rebaseFilesHelper(repo),
      applyDiff(repo),
      index(repo)
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

    indexWorktree.resetIndexToTree(rebaseFilesHelper.getOrigHead());
    indexWorktree.copyForceIndexToWorktree();
    refs.updateSymbolicRef("HEAD", rebaseFilesHelper.getHeadName());

    rebaseFilesHelper.deleteAllRebaseFiles();

    return Error::NO_ERROR;
}

auto Rebase::continueRebase() const -> std::expected<std::string, Error>
{
    return continueRebase("", "");
}

auto Rebase::continueRebase(const std::string_view message, const std::string_view description) const -> std::expected<std::string, Error>
{
    if (!isRebaseInProgress())
    {
        return std::unexpected{ Error::NO_REBASE_IN_PROGRESS };
    }

    auto continueResult = std::expected<std::string, Error>{};

    auto lastCommand = rebaseFilesHelper.getLastDoneCommand();

    if (auto amend = rebaseFilesHelper.getAmendFile(); amend != "")
    {
        continueResult = continueEditImpl();
    }
    else if (lastCommand->type == RebaseTodoCommandType::REWORD)
    {
        continueResult = continueRewordImpl(lastCommand.value(), message, description);
    }
    else if (rebaseFilesHelper.areAnySquashInCurrentFixup())
    {
        continueResult = continueSquashImpl(message, description);
    }
    else if (auto stoppedSha = rebaseFilesHelper.getStoppedShaFile(); stoppedSha != "")
    {
        continueResult = continueConflictImpl(stoppedSha);
    }

    if (!continueResult.has_value())
    {
        return continueResult;
    }


    if (auto processTodoListResult = processTodoList(); processTodoListResult != Error::NO_ERROR)
    {
        return std::unexpected{ processTodoListResult };
    }

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

auto Rebase::getSquashMessage() const -> std::string
{
    return rebaseFilesHelper.getMessageSqaushFile();
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

    rebaseFilesHelper.createRebaseDir();
    rebaseFilesHelper.createHeadNameFile(branches.getCurrentBranch());
    rebaseFilesHelper.createOntoFile(upstreamHash);
    rebaseFilesHelper.createOrigHeadFiles(refs.getRefHash("HEAD"));
    rebaseFilesHelper.generateTodoFile(rebaseCommands);

    branches.detachHead(upstreamHash);
}

auto Rebase::endRebase() const -> std::string
{
    auto currentHash = refs.getRefHash("HEAD");
    auto headName = rebaseFilesHelper.getHeadName();
    refs.updateRefHash(headName, currentHash);
    refs.updateSymbolicRef("HEAD", headName);

    rebaseFilesHelper.deleteAllRebaseFiles();

    return currentHash;
}

auto Rebase::processTodoList() const -> Error
{
    auto todoCommand = rebaseFilesHelper.peakAndPopTodoFile();
    while (todoCommand)
    {
        auto todoResult = processTodoCommand(todoCommand.value());
        rebaseFilesHelper.appendDoneFile(todoCommand.value());

        if (todoResult == Error::REBASE_CONFLICT)
        {
            startConflict(todoCommand.value());
            return Error::REBASE_CONFLICT;
        }

        if (todoResult != Error::NO_ERROR)
        {
            return todoResult;
        }

        todoCommand = rebaseFilesHelper.peakAndPopTodoFile();
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

    if (rebaseTodoCommand.type == RebaseTodoCommandType::REWORD)
    {
        return processReword(rebaseTodoCommand);
    }

    if (rebaseTodoCommand.type == RebaseTodoCommandType::EDIT)
    {
        return processEdit(rebaseTodoCommand);
    }

    if (rebaseTodoCommand.type == RebaseTodoCommandType::DROP)
    {
        return processDrop(rebaseTodoCommand);
    }

    if (rebaseTodoCommand.type == RebaseTodoCommandType::FIXUP)
    {
        return processFixup(rebaseTodoCommand);
    }

    if (rebaseTodoCommand.type == RebaseTodoCommandType::SQUASH)
    {
        return processSquash(rebaseTodoCommand);
    }

    throw std::runtime_error("Todo command not yet implemented");
}

auto Rebase::processPickCommand(const RebaseTodoCommand& rebaseTodoCommand) const -> Error
{
    auto pickResult = pickCommit(rebaseTodoCommand);

    if (!pickResult.has_value())
    {
        return pickResult.error();
    }

    const auto& newCommitHash = pickResult.value();

    if (isNextCommandFixupOrSquash())
    {
        rebaseFilesHelper.appendRewrittenPendingFile(rebaseTodoCommand.hash);
        auto commitInfo = Commits{ repo }.getCommitInfo(rebaseTodoCommand.hash);
    }
    else if (rebaseTodoCommand.hash != newCommitHash)
    {
        rebaseFilesHelper.appendRewrittenListFile(rebaseTodoCommand.hash, newCommitHash);
    }

    return Error::NO_ERROR;
}

auto Rebase::processBreakCommand(const RebaseTodoCommand&) const -> Error
{
    return Error::REBASE_BREAK;
}

auto Rebase::processReword(const RebaseTodoCommand& rebaseTodoCommand) const -> Error
{
    auto pickResult = pickCommit(rebaseTodoCommand).error_or(Error::NO_ERROR);

    if (pickResult != Error::NO_ERROR)
    {
        return pickResult;
    }

    auto commits = Commits{ repo };
    auto commitInfo = commits.getCommitInfo(rebaseTodoCommand.hash);

    auto msgAndDesc = concatMessageAndDescription(commitInfo.getMessage(), commitInfo.getDescription());
    rebaseFilesHelper.createCommitEditMsgFile(commitInfo.getMessage());
    rebaseFilesHelper.createAuthorScriptFile(commitInfo.getAuthor().name, commitInfo.getAuthor().email, commitInfo.getAuthorDate());

    return Error::REBASE_REWORD;
}

auto Rebase::processEdit(const RebaseTodoCommand& rebaseTodoCommand) const -> Error
{
    rebaseFilesHelper.createRebaseHeadFile(rebaseTodoCommand.hash);
    auto pickResult = pickCommit(rebaseTodoCommand).error_or(Error::NO_ERROR);

    if (pickResult != Error::NO_ERROR)
    {
        return pickResult;
    }

    auto commits = Commits{ repo };
    auto commitInfo = commits.getCommitInfo(rebaseTodoCommand.hash);

    auto msgAndDesc = concatMessageAndDescription(commitInfo.getMessage(), commitInfo.getDescription());
    rebaseFilesHelper.createAuthorScriptFile(commitInfo.getAuthor().name, commitInfo.getAuthor().email, commitInfo.getAuthorDate());

    auto hashCommit = commits.getHeadCommitHash();
    rebaseFilesHelper.createAmendFile(hashCommit);
    rebaseFilesHelper.createStoppedShaFile(rebaseTodoCommand.hash);

    return Error::REBASE_EDIT;
}

auto Rebase::processDrop(const RebaseTodoCommand&) const -> Error
{
    return Error::NO_ERROR;
}

auto Rebase::processFixup(const RebaseTodoCommand& rebaseTodoCommand) const -> Error
{
    auto applyResult = applyDiff.apply(rebaseTodoCommand.hash);

    if (applyResult == _details::ApplyDiffResult::CONFLICT)
    {
        // TODO what to do when fixup has conflict
        return Error::REBASE_CONFLICT;
    }

    rebaseFilesHelper.appendCurrentFixupFile(rebaseTodoCommand);

    if (!isNextCommandFixupOrSquash())
    {
        if (rebaseFilesHelper.areAnySquashInCurrentFixup())
        {
            return Error::REBASE_SQUASH;
        }
        else
        {
            auto newCommitHash = Commits{ repo }.amendCommit();
            rebaseFilesHelper.appendRewrittenPendingFile(rebaseTodoCommand.hash);

            rebaseFilesHelper.moveRewrittenPendingToRewrittenList(newCommitHash);
            rebaseFilesHelper.removeCurrentFixupFile();
            return Error::NO_ERROR;
        }
    }

    auto commits = Commits{ repo };

    auto commitInfo = commits.getCommitInfo(rebaseTodoCommand.hash);
    auto headCommitInfo = commits.getCommitInfo(commits.getHeadCommitHash());

    auto messageSquash = headCommitInfo.getMessage();
    messageSquash += headCommitInfo.getDescription().empty() ? "" : "\n\n" + headCommitInfo.getDescription();
    messageSquash += "\n\n" + commitInfo.getMessage();
    messageSquash += commitInfo.getDescription().empty() ? "" : "\n\n" + commitInfo.getDescription();
    rebaseFilesHelper.createMessageSquashFile(messageSquash);

    rebaseFilesHelper.appendRewrittenPendingFile(rebaseTodoCommand.hash);
    auto newCommitHash = Commits{ repo }.amendCommit();
    return Error::NO_ERROR;
}

auto Rebase::processSquash(const RebaseTodoCommand& rebaseTodoCommand) const -> Error
{
    auto applyResult = applyDiff.apply(rebaseTodoCommand.hash);

    if (applyResult == _details::ApplyDiffResult::CONFLICT)
    {
        // TODO what to do when fixup has conflict
        return Error::REBASE_CONFLICT;
    }
    auto commits = Commits{ repo };

    rebaseFilesHelper.appendCurrentFixupFile(rebaseTodoCommand);

    auto commitInfo = commits.getCommitInfo(rebaseTodoCommand.hash);
    auto headCommitInfo = commits.getCommitInfo(commits.getHeadCommitHash());

    auto messageSquash = headCommitInfo.getMessage();
    messageSquash += headCommitInfo.getDescription().empty() ? "" : "\n\n" + headCommitInfo.getDescription();
    messageSquash += "\n\n" + commitInfo.getMessage();
    messageSquash += commitInfo.getDescription().empty() ? "" : "\n\n" + commitInfo.getDescription();
    rebaseFilesHelper.createMessageSquashFile(messageSquash);

    if (!isNextCommandFixupOrSquash())
    {
        return Error::REBASE_SQUASH;
    }

    auto newCommitHash = commits.amendCommit(messageSquash);
    rebaseFilesHelper.appendRewrittenPendingFile(rebaseTodoCommand.hash);

    return Error::NO_ERROR;
}

auto Rebase::pickCommit(const RebaseTodoCommand& rebaseTodoCommand) const -> std::expected<std::string, Error>
{
    auto commits = Commits{ repo };
    auto headCommitHash = commits.getHeadCommitHash();

    if (auto pickedCommitInfo = commits.getCommitInfo(rebaseTodoCommand.hash); pickedCommitInfo.getParents()[0] == headCommitHash)
    {
        // can FastForward
        indexWorktree.resetIndexToTree(rebaseTodoCommand.hash);
        indexWorktree.copyForceIndexToWorktree();
        refs.detachHead(rebaseTodoCommand.hash);

        return rebaseTodoCommand.hash;
    }

    auto cherryPickResult = cherryPick.cherryPickCommit(rebaseTodoCommand.hash, CherryPickEmptyCommitStrategy::KEEP);

    if (!cherryPickResult.has_value())
    {
        auto error = cherryPickResult.error();

        if (error == Error::CHERRY_PICK_CONFLICT)
        {
            return std::unexpected{ Error::REBASE_CONFLICT };
        }
        else
        {
            throw std::runtime_error("Unexpected error during cherry-pick");
        }
    }

    return cherryPickResult.value();
}


auto Rebase::startConflict(const RebaseTodoCommand& rebaseTodoCommand) const -> void
{
    rebaseFilesHelper.createStoppedShaFile(rebaseTodoCommand.hash);
}

auto Rebase::isNextCommandFixupOrSquash() const -> bool
{
    auto peakCommand = rebaseFilesHelper.peekTodoFile();

    return peakCommand && (peakCommand->type == RebaseTodoCommandType::FIXUP || peakCommand->type == RebaseTodoCommandType::SQUASH);
}

auto Rebase::continueEditImpl() const -> std::expected<std::string, Error>
{
    auto commits = Commits{ repo };
    auto hashBefore = rebaseFilesHelper.getRebaseHeadFile();
    auto hashAfter = hashBefore;

    if (index.areAnyNotStagedTrackedFiles())
    {
        return std::unexpected{ Error::DIRTY_WORKTREE };
    }

    if (index.areAnyStagedFiles())
    {
        hashAfter = commits.amendCommit();
    }

    rebaseFilesHelper.appendRewrittenListFile(hashBefore, hashAfter);
    rebaseFilesHelper.removeAmendFile();
    rebaseFilesHelper.removeStoppedShaFile();
    rebaseFilesHelper.removeAuthorScriptFile();

    return hashAfter;
}

auto Rebase::continueRewordImpl(const RebaseTodoCommand& lastDoneCommand, const std::string_view message, const std::string_view description) const -> std::expected<std::string, Error>
{
    auto messageAndDesc = std::string{};

    if (message.empty())
    {
        messageAndDesc = rebaseFilesHelper.getCommitEditMsgFile();
    }
    else
    {
        messageAndDesc = concatMessageAndDescription(message, description);
    }

    auto commitHash = Commits{ repo }.amendCommit(messageAndDesc);
    rebaseFilesHelper.appendRewrittenListFile(lastDoneCommand.hash, commitHash);

    return commitHash;
}

auto Rebase::continueSquashImpl(const std::string_view message, const std::string_view description) const -> std::expected<std::string, Error>
{
    auto messageAndDesc = std::string{};

    if (message.empty())
    {
        messageAndDesc = rebaseFilesHelper.getMessageSqaushFile();
    }
    else
    {
        messageAndDesc = concatMessageAndDescription(message, description);
    }

    auto commitHash = Commits{ repo }.amendCommit(messageAndDesc);
    auto lastCommand = rebaseFilesHelper.getLastDoneCommand();
    rebaseFilesHelper.appendRewrittenPendingFile(lastCommand->hash);
    rebaseFilesHelper.moveRewrittenPendingToRewrittenList(commitHash);
    rebaseFilesHelper.removeCurrentFixupFile();
    rebaseFilesHelper.removeMessageSqaushFile();

    return commitHash;
}

auto Rebase::continueConflictImpl(const std::string_view stoppedSha) const -> std::expected<std::string, Error>
{
    auto commits = Commits{ repo };
    auto _createCommit = _details::CreateCommit{ repo };
    auto commitInfo = commits.getCommitInfo(stoppedSha);

    auto envp = std::vector<std::string>{
        "GIT_AUTHOR_NAME=" + commitInfo.getAuthor().name,
        "GIT_AUTHOR_EMAIL=" + commitInfo.getAuthor().email,
        "GIT_AUTHOR_DATE=" + commitInfo.getAuthorDate()
    };

    auto parent = commits.hasAnyCommits() ? commits.getHeadCommitHash() : std::string{};

    auto hashAfter = _createCommit.createCommit(commitInfo.getMessage(), commitInfo.getDescription(), { parent }, envp);
    rebaseFilesHelper.appendRewrittenListFile(stoppedSha, hashAfter);
    rebaseFilesHelper.removeStoppedShaFile();

    return hashAfter;
}

auto Rebase::concatMessageAndDescription(const std::string_view message, const std::string_view description) const -> std::string
{
    auto messageAndDesc = std::string{ message };
    if (!description.empty())
    {
        messageAndDesc += "\n\n";
        messageAndDesc += description;
    }

    return messageAndDesc;
}

} // namespace CppGit

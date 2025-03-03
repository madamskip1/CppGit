#include "Rebase.hpp"

#include "Branches.hpp"
#include "Commit.hpp"
#include "Commits.hpp"
#include "CommitsHistory.hpp"
#include "Error.hpp"
#include "Index.hpp"
#include "RebaseTodoCommand.hpp"
#include "Repository.hpp"
#include "_details/AmendCommit.hpp"
#include "_details/ApplyDiff.hpp"
#include "_details/CreateCommit.hpp"
#include "_details/GitFilesHelper.hpp"

#include <algorithm>
#include <expected>
#include <filesystem>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace CppGit {
Rebase::Rebase(const Repository& repo)
    : repo{ repo },
      commits{ repo },
      refs{ repo },
      indexWorktree{ repo },
      rebaseFilesHelper{ repo },
      applyDiff{ repo }
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

    if (auto lastCommand = rebaseFilesHelper.getLastDoneCommand(); lastCommand->type != RebaseTodoCommandType::BREAK)
    {
        auto messageAndDesc = std::string{};
        if (!message.empty())
        {
            messageAndDesc = std::string{ message };
            if (!description.empty())
            {
                messageAndDesc += "\n\n" + std::string{ description };
            }
        }
        else
        {
            messageAndDesc = rebaseFilesHelper.getMessageFile();
        }

        auto hashBefore = rebaseFilesHelper.getRebaseHeadFile();
        auto hashAfter = std::string{};

        if (auto amend = rebaseFilesHelper.getAmendFile(); !amend.empty())
        {
            hashAfter = commits.getHeadCommitHash();

            if (lastCommand->type != RebaseTodoCommandType::EDIT || Index{ repo }.areAnyStagedFiles())
            {
                const auto headCommitInfo = commits.getCommitInfo(hashAfter);
                hashAfter = _details::AmendCommit{ repo }.amend(headCommitInfo, messageAndDesc);
                refs.updateRefHash("HEAD", hashAfter);
            }

            rebaseFilesHelper.removeAmendFile();
        }
        else
        {
            const auto authorScript = rebaseFilesHelper.getAuthorScriptFile(); // authorScript has struct like envp
            const auto parent = commits.getHeadCommitHash();

            hashAfter = _details::CreateCommit{ repo }.createCommit(messageAndDesc, { parent }, authorScript);
            refs.updateRefHash("HEAD", hashAfter);
            rebaseFilesHelper.removeAuthorScriptFile();
        }

        // We remove it because, for example, if a conflict occurs, we have already performed squash-like operations by naming commits.
        // This ensures that if only fixups happen afterward (and no squash), the user won't be prompted to change the message.
        rebaseFilesHelper.removeCurrentFixupFile();

        if (isNextCommandFixupOrSquash())
        {
            rebaseFilesHelper.appendRewrittenPendingFile(hashBefore);
        }
        else
        {
            rebaseFilesHelper.appendRewrittenListWithRewrittenPending(hashAfter);
            rebaseFilesHelper.appendRewrittenListFile(hashBefore, hashAfter);
        }

        rebaseFilesHelper.removeMessageFile();
        rebaseFilesHelper.removeRebaseHeadFile();
    }

    if (const auto processTodoListResult = processTodoList(); processTodoListResult != Error::NO_ERROR)
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
    const auto rebaseBase = repo.executeGitCommand("merge-base", "HEAD", upstream);

    if (rebaseBase.return_code != 0)
    {
        throw std::runtime_error("Failed to find merge base");
    }

    auto commitsHistory = repo.CommitsHistory();
    commitsHistory.setOrder(CommitsHistory::Order::REVERSE);
    auto commitsToRebase = commitsHistory.getCommitsLogDetailed(std::move(rebaseBase.stdout), "HEAD");

    auto rebaseCommands = std::vector<RebaseTodoCommand>{};

    std::ranges::transform(commitsToRebase,
                           std::back_inserter(rebaseCommands),
                           [](auto& commit) {
                               return RebaseTodoCommand{ RebaseTodoCommandType::PICK, std::move(commit.getHash()), std::move(commit.getMessage()) };
                           });

    return rebaseCommands;
}

auto Rebase::getStoppedMessage() const -> std::string
{
    return rebaseFilesHelper.getMessageFile();
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
    const auto branches = repo.Branches();
    const auto upstreamHash = refs.getRefHash(upstream);
    const auto headRef = refs.getRefHash("HEAD");

    rebaseFilesHelper.createRebaseDir();
    rebaseFilesHelper.createHeadNameFile(branches.getCurrentBranchName());
    rebaseFilesHelper.createOntoFile(upstreamHash);
    rebaseFilesHelper.createRebaseOrigHeadFile(headRef);
    _details::GitFilesHelper{ repo }.setOrigHeadFile(headRef);
    rebaseFilesHelper.generateTodoFile(rebaseCommands);

    branches.detachHead(upstreamHash);
}

auto Rebase::endRebase() const -> std::string
{
    const auto currentHash = refs.getRefHash("HEAD");
    const auto headName = rebaseFilesHelper.getHeadName();
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
        auto todoCommandValue = todoCommand.value();

        if (todoCommandValue.type != RebaseTodoCommandType::BREAK)
        {
            rebaseFilesHelper.createRebaseHeadFile(todoCommandValue.hash);
        }

        const auto todoResult = processTodoCommand(todoCommandValue);
        rebaseFilesHelper.appendDoneFile(todoCommandValue);

        if (todoResult != Error::NO_ERROR)
        {
            return todoResult;
        }

        rebaseFilesHelper.removeRebaseHeadFile();

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
    const auto commitInfo = commits.getCommitInfo(rebaseTodoCommand.hash);
    const auto pickResult = pickCommit(commitInfo);

    if (!pickResult.has_value())
    {
        rebaseFilesHelper.createAuthorScriptFile(commitInfo.getAuthor().name, commitInfo.getAuthor().email, commitInfo.getAuthorDate());
        rebaseFilesHelper.createMessageFile(commitInfo.getMessageAndDescription());
        return pickResult.error();
    }

    if (isNextCommandFixupOrSquash())
    {
        rebaseFilesHelper.appendRewrittenPendingFile(rebaseTodoCommand.hash);
    }
    else if (const auto& newCommitHash = pickResult.value(); rebaseTodoCommand.hash != newCommitHash)
    {
        rebaseFilesHelper.appendRewrittenListFile(rebaseTodoCommand.hash, newCommitHash);
    }

    return Error::NO_ERROR;
}

auto Rebase::processBreakCommand(const RebaseTodoCommand& /*rebaseTodoCommand*/) -> Error
{
    return Error::REBASE_BREAK;
}

auto Rebase::processReword(const RebaseTodoCommand& rebaseTodoCommand) const -> Error
{
    const auto commitInfo = commits.getCommitInfo(rebaseTodoCommand.hash);
    const auto pickResult = pickCommit(commitInfo);

    rebaseFilesHelper.createMessageFile(commitInfo.getMessageAndDescription());

    if (!pickResult.has_value())
    {
        rebaseFilesHelper.createAuthorScriptFile(commitInfo.getAuthor().name, commitInfo.getAuthor().email, commitInfo.getAuthorDate());
        return pickResult.error();
    }

    rebaseFilesHelper.createAmendFile(pickResult.value());

    return Error::REBASE_REWORD;
}

auto Rebase::processEdit(const RebaseTodoCommand& rebaseTodoCommand) const -> Error
{
    const auto commitInfo = commits.getCommitInfo(rebaseTodoCommand.hash);
    const auto pickResult = pickCommit(commitInfo);

    rebaseFilesHelper.createMessageFile(commitInfo.getMessageAndDescription());

    if (!pickResult.has_value())
    {
        rebaseFilesHelper.createAuthorScriptFile(commitInfo.getAuthor().name, commitInfo.getAuthor().email, commitInfo.getAuthorDate());
        return pickResult.error();
    }

    rebaseFilesHelper.createAmendFile(pickResult.value());

    return Error::REBASE_EDIT;
}

auto Rebase::processDrop(const RebaseTodoCommand& /*rebaseTodoCommand*/) -> Error
{
    return Error::NO_ERROR;
}

auto Rebase::processFixup(const RebaseTodoCommand& rebaseTodoCommand) const -> Error
{
    const auto headCommitHash = commits.getHeadCommitHash();
    const auto headCommitInfo = commits.getCommitInfo(headCommitHash);

    if (auto applyResult = applyDiff.apply(rebaseTodoCommand.hash); applyResult == _details::ApplyDiffResult::CONFLICT)
    {
        rebaseFilesHelper.createAmendFile(headCommitHash);
        rebaseFilesHelper.appendCurrentFixupFile(rebaseTodoCommand);
        rebaseFilesHelper.createMessageFile(headCommitInfo.getMessageAndDescription());

        return Error::REBASE_CONFLICT;
    }

    if (isNextCommandFixupOrSquash())
    {
        rebaseFilesHelper.appendCurrentFixupFile(rebaseTodoCommand);
        rebaseFilesHelper.appendRewrittenPendingFile(rebaseTodoCommand.hash);

        const auto newCommitHash = _details::AmendCommit{ repo }.amend(headCommitInfo);
        refs.updateRefHash("HEAD", newCommitHash);

        return Error::NO_ERROR;
    }

    if (rebaseFilesHelper.areAnySquashInCurrentFixup())
    {
        rebaseFilesHelper.createAmendFile(headCommitHash);
        rebaseFilesHelper.appendCurrentFixupFile(rebaseTodoCommand);
        rebaseFilesHelper.createMessageFile(headCommitInfo.getMessageAndDescription());

        return Error::REBASE_SQUASH;
    }

    const auto newCommitHash = _details::AmendCommit{ repo }.amend(headCommitInfo);
    refs.updateRefHash("HEAD", newCommitHash);

    rebaseFilesHelper.appendRewrittenPendingFile(rebaseTodoCommand.hash);
    rebaseFilesHelper.appendRewrittenListWithRewrittenPending(newCommitHash);
    rebaseFilesHelper.removeCurrentFixupFile();

    return Error::NO_ERROR;
}

auto Rebase::processSquash(const RebaseTodoCommand& rebaseTodoCommand) const -> Error
{
    const auto messageSquash = getConcatenatedMessagePreviousAndCurrentCommit(commits.getHeadCommitHash(), rebaseTodoCommand.hash);

    if (const auto applyResult = applyDiff.apply(rebaseTodoCommand.hash); applyResult == _details::ApplyDiffResult::CONFLICT)
    {
        rebaseFilesHelper.createAmendFile(commits.getHeadCommitHash());
        rebaseFilesHelper.createMessageFile(messageSquash);
        rebaseFilesHelper.appendCurrentFixupFile(rebaseTodoCommand);

        return Error::REBASE_CONFLICT;
    }

    if (!isNextCommandFixupOrSquash())
    {
        rebaseFilesHelper.createAmendFile(commits.getHeadCommitHash());
        rebaseFilesHelper.createMessageFile(messageSquash);

        return Error::REBASE_SQUASH;
    }

    rebaseFilesHelper.appendRewrittenPendingFile(rebaseTodoCommand.hash);
    rebaseFilesHelper.appendCurrentFixupFile(rebaseTodoCommand);

    const auto headCommitHash = commits.getHeadCommitHash();
    const auto headCommitInfo = commits.getCommitInfo(headCommitHash);
    const auto newCommitHash = _details::AmendCommit{ repo }.amend(headCommitInfo, messageSquash);
    refs.updateRefHash("HEAD", newCommitHash);

    return Error::NO_ERROR;
}

auto Rebase::pickCommit(const Commit& commitInfo) const -> std::expected<std::string, Error>
{
    const auto& pickedParent = commitInfo.getParents()[0];
    const auto headCommitHash = commits.getHeadCommitHash();

    if (headCommitHash == pickedParent)
    {
        // can FastForward
        indexWorktree.resetIndexToTree(commitInfo.getHash());
        indexWorktree.copyForceIndexToWorktree();
        refs.detachHead(commitInfo.getHash());

        return commitInfo.getHash();
    }

    const auto applyDiffResult = applyDiff.apply(commitInfo.getHash());

    if (applyDiffResult == _details::ApplyDiffResult::EMPTY_DIFF || applyDiffResult == _details::ApplyDiffResult::NO_CHANGES)
    {
        // TODO: what to do with empty commits?
    }

    if (applyDiffResult == _details::ApplyDiffResult::CONFLICT)
    {
        return std::unexpected{ Error::REBASE_CONFLICT };
    }

    const auto envp = std::vector<std::string>{
        "GIT_AUTHOR_NAME=" + commitInfo.getAuthor().name,
        "GIT_AUTHOR_EMAIL=" + commitInfo.getAuthor().email,
        "GIT_AUTHOR_DATE=" + commitInfo.getAuthorDate()
    };

    auto newCommitHash = _details::CreateCommit{ repo }.createCommit(commitInfo.getMessage(), commitInfo.getDescription(), { headCommitHash }, envp);
    refs.updateRefHash("HEAD", newCommitHash);

    return newCommitHash;
}

auto Rebase::isNextCommandFixupOrSquash() const -> bool
{
    const auto peakCommand = rebaseFilesHelper.peekTodoFile();

    return peakCommand && (peakCommand->type == RebaseTodoCommandType::FIXUP || peakCommand->type == RebaseTodoCommandType::SQUASH);
}

auto Rebase::getConcatenatedMessagePreviousAndCurrentCommit(const std::string_view previousCommitHash, const std::string_view currentCommitHash) const -> std::string
{
    const auto previousCommitInfo = commits.getCommitInfo(previousCommitHash);
    const auto currentCommitInfo = commits.getCommitInfo(currentCommitHash);

    return previousCommitInfo.getMessageAndDescription() + "\n\n" + currentCommitInfo.getMessageAndDescription();
}
} // namespace CppGit

#include "CppGit/Rebase.hpp"

#include "CppGit/Branches.hpp"
#include "CppGit/Commit.hpp"
#include "CppGit/Commits.hpp"
#include "CppGit/CommitsHistory.hpp"
#include "CppGit/Index.hpp"
#include "CppGit/RebaseTodoCommand.hpp"
#include "CppGit/Repository.hpp"
#include "CppGit/_details/AmendCommit.hpp"
#include "CppGit/_details/ApplyDiff.hpp"
#include "CppGit/_details/CreateCommit.hpp"
#include "CppGit/_details/GitFilesHelper.hpp"

#include <algorithm>
#include <expected>
#include <filesystem>
#include <iterator>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace CppGit {
Rebase::Rebase(const Repository& repo)
    : repo{ &repo },
      commits{ repo },
      branches{ repo },
      refs{ repo },
      indexWorktree{ repo },
      rebaseFilesHelper{ repo },
      applyDiff{ repo },
      amendCommit{ repo },
      createCommit{ repo }
{
}

auto Rebase::rebase(const std::string_view upstream) const -> std::expected<std::string, RebaseResult>
{
    auto rebaseComamnds = getDefaultTodoCommands(upstream);
    return rebaseImpl(upstream, rebaseComamnds);
}

auto Rebase::interactiveRebase(const std::string_view upstream, const std::vector<RebaseTodoCommand>& rebaseCommands) const -> std::expected<std::string, RebaseResult>
{
    return rebaseImpl(upstream, rebaseCommands);
}

auto Rebase::abortRebase() const -> void
{
    indexWorktree.resetIndexToTree(rebaseFilesHelper.getOrigHead());
    indexWorktree.copyForceIndexToWorktree();
    refs.updateSymbolicRef("HEAD", rebaseFilesHelper.getHeadName());

    rebaseFilesHelper.deleteAllRebaseFiles();
}

auto Rebase::continueRebase() const -> std::expected<std::string, RebaseResult>
{
    return continueRebase("", "");
}

auto Rebase::continueRebase(const std::string_view message, const std::string_view description) const -> std::expected<std::string, RebaseResult>
{
    if (auto lastCommand = rebaseFilesHelper.getLastDoneCommand(); lastCommand->type != RebaseTodoCommandType::BREAK)
    {
        const auto messageAndDesc = [&message, &description, this] {
            if (!message.empty())
            {
                if (!description.empty())
                {
                    return std::string{ message } + "\n\n" + std::string{ description };
                }

                return std::string{ message };
            }

            return rebaseFilesHelper.getMessageFile();
        }();

        auto hashBefore = rebaseFilesHelper.getRebaseHeadFile();
        auto hashAfter = std::string{};

        if (auto amend = rebaseFilesHelper.getAmendFile(); !amend.empty())
        {
            hashAfter = commits.getHeadCommitHash();

            if (lastCommand->type != RebaseTodoCommandType::EDIT || repo->Index().areAnyStagedFiles())
            {
                const auto headCommitInfo = commits.getCommitInfo(hashAfter);
                hashAfter = amendCommit.amend(headCommitInfo, messageAndDesc);
                refs.updateRefHash("HEAD", hashAfter);
            }

            rebaseFilesHelper.removeAmendFile();
        }
        else
        {
            const auto authorScript = rebaseFilesHelper.getAuthorScriptFile(); // authorScript has struct like envp
            const auto parent = commits.getHeadCommitHash();

            hashAfter = createCommit.createCommit(messageAndDesc, { parent }, authorScript);
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

    if (const auto processTodoListResult = processTodoList(); processTodoListResult != RebaseResult::COMMAND_PROCESSED)
    {
        return std::unexpected{ processTodoListResult };
    }

    return endRebase();
}

auto Rebase::isRebaseInProgress() const -> bool
{
    return std::filesystem::exists(repo->getGitDirectoryPath() / "rebase-merge" / "git-rebase-todo");
}

auto Rebase::getDefaultTodoCommands(const std::string_view upstream) const -> std::vector<RebaseTodoCommand>
{
    auto rebaseBase = repo->executeGitCommand("merge-base", "HEAD", upstream);

    auto commitsHistory = repo->CommitsHistory();
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

auto Rebase::rebaseImpl(const std::string_view upstream, const std::vector<RebaseTodoCommand>& rebaseCommands) const -> std::expected<std::string, RebaseResult>
{
    startRebase(upstream, rebaseCommands);

    if (auto todoResult = processTodoList(); todoResult != RebaseResult::COMMAND_PROCESSED)
    {
        return std::unexpected{ todoResult };
    }

    return endRebase();
}

auto Rebase::startRebase(const std::string_view upstream, const std::vector<RebaseTodoCommand>& rebaseCommands) const -> void
{
    const auto upstreamHash = refs.getRefHash(upstream);
    const auto headRef = refs.getRefHash("HEAD");

    rebaseFilesHelper.createRebaseDir();
    rebaseFilesHelper.createHeadNameFile(branches.getCurrentBranchName());
    rebaseFilesHelper.createOntoFile(upstreamHash);
    rebaseFilesHelper.createRebaseOrigHeadFile(headRef);
    _details::GitFilesHelper{ *repo }.setOrigHeadFile(headRef);
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

auto Rebase::processTodoList() const -> RebaseResult
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

        if (todoResult != RebaseResult::COMMAND_PROCESSED && todoResult != RebaseResult::EMPTY_DIFF)
        {
            return todoResult;
        }

        rebaseFilesHelper.removeRebaseHeadFile();

        todoCommand = rebaseFilesHelper.peakAndPopTodoFile();
    }

    return RebaseResult::COMMAND_PROCESSED;
}

auto Rebase::processTodoCommand(const RebaseTodoCommand& rebaseTodoCommand) const -> RebaseResult
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

    return RebaseResult::UNKNOWN_COMMAND;
}

auto Rebase::processPickCommand(const RebaseTodoCommand& rebaseTodoCommand) const -> RebaseResult
{
    const auto commitInfo = commits.getCommitInfo(rebaseTodoCommand.hash);
    const auto pickResult = pickCommit(commitInfo);

    if (!pickResult.has_value())
    {
        auto error = pickResult.error();

        if (error == RebaseResult::EMPTY_DIFF)
        {
            return error;
        }

        rebaseFilesHelper.createAuthorScriptFile(commitInfo.getAuthor().name, commitInfo.getAuthor().email, commitInfo.getAuthorDate());
        rebaseFilesHelper.createMessageFile(commitInfo.getMessageAndDescription());

        return error;
    }

    if (isNextCommandFixupOrSquash())
    {
        rebaseFilesHelper.appendRewrittenPendingFile(rebaseTodoCommand.hash);
    }
    else if (const auto& newCommitHash = pickResult.value(); rebaseTodoCommand.hash != newCommitHash)
    {
        rebaseFilesHelper.appendRewrittenListFile(rebaseTodoCommand.hash, newCommitHash);
    }

    return RebaseResult::COMMAND_PROCESSED;
}

auto Rebase::processBreakCommand(const RebaseTodoCommand& /*rebaseTodoCommand*/) -> RebaseResult
{
    return RebaseResult::BREAK;
}

auto Rebase::processReword(const RebaseTodoCommand& rebaseTodoCommand) const -> RebaseResult
{
    const auto commitInfo = commits.getCommitInfo(rebaseTodoCommand.hash);
    const auto pickResult = pickCommit(commitInfo);

    rebaseFilesHelper.createMessageFile(commitInfo.getMessageAndDescription());

    if (!pickResult.has_value() && pickResult.error() != RebaseResult::EMPTY_DIFF)
    {
        rebaseFilesHelper.createAuthorScriptFile(commitInfo.getAuthor().name, commitInfo.getAuthor().email, commitInfo.getAuthorDate());
        return pickResult.error();
    }

    rebaseFilesHelper.createAmendFile(pickResult.value());

    return RebaseResult::REWORD;
}

auto Rebase::processEdit(const RebaseTodoCommand& rebaseTodoCommand) const -> RebaseResult
{
    const auto commitInfo = commits.getCommitInfo(rebaseTodoCommand.hash);
    const auto pickResult = pickCommit(commitInfo);

    rebaseFilesHelper.createMessageFile(commitInfo.getMessageAndDescription());

    if (!pickResult.has_value() && pickResult.error() != RebaseResult::EMPTY_DIFF)
    {
        rebaseFilesHelper.createAuthorScriptFile(commitInfo.getAuthor().name, commitInfo.getAuthor().email, commitInfo.getAuthorDate());
        return pickResult.error();
    }

    rebaseFilesHelper.createAmendFile(pickResult.value());

    return RebaseResult::EDIT;
}

auto Rebase::processDrop(const RebaseTodoCommand& /*rebaseTodoCommand*/) -> RebaseResult
{
    return RebaseResult::COMMAND_PROCESSED;
}

auto Rebase::processFixup(const RebaseTodoCommand& rebaseTodoCommand) const -> RebaseResult
{
    const auto headCommitHash = commits.getHeadCommitHash();
    const auto headCommitInfo = commits.getCommitInfo(headCommitHash);

    if (auto applyResult = applyDiff.apply(rebaseTodoCommand.hash); applyResult == _details::ApplyDiffResult::CONFLICT)
    {
        rebaseFilesHelper.createAmendFile(headCommitHash);
        rebaseFilesHelper.appendCurrentFixupFile(rebaseTodoCommand);
        rebaseFilesHelper.createMessageFile(headCommitInfo.getMessageAndDescription());

        return RebaseResult::CONFLICT;
    }

    if (isNextCommandFixupOrSquash())
    {
        rebaseFilesHelper.appendCurrentFixupFile(rebaseTodoCommand);
        rebaseFilesHelper.appendRewrittenPendingFile(rebaseTodoCommand.hash);

        const auto newCommitHash = amendCommit.amend(headCommitInfo);
        refs.updateRefHash("HEAD", newCommitHash);

        return RebaseResult::COMMAND_PROCESSED;
    }

    if (rebaseFilesHelper.areAnySquashInCurrentFixup())
    {
        rebaseFilesHelper.createAmendFile(headCommitHash);
        rebaseFilesHelper.appendCurrentFixupFile(rebaseTodoCommand);
        rebaseFilesHelper.createMessageFile(headCommitInfo.getMessageAndDescription());

        return RebaseResult::SQUASH;
    }

    const auto newCommitHash = amendCommit.amend(headCommitInfo);
    refs.updateRefHash("HEAD", newCommitHash);

    rebaseFilesHelper.appendRewrittenPendingFile(rebaseTodoCommand.hash);
    rebaseFilesHelper.appendRewrittenListWithRewrittenPending(newCommitHash);
    rebaseFilesHelper.removeCurrentFixupFile();

    return RebaseResult::COMMAND_PROCESSED;
}

auto Rebase::processSquash(const RebaseTodoCommand& rebaseTodoCommand) const -> RebaseResult
{
    const auto messageSquash = getConcatenatedMessagePreviousAndCurrentCommit(commits.getHeadCommitHash(), rebaseTodoCommand.hash);

    if (const auto applyResult = applyDiff.apply(rebaseTodoCommand.hash); applyResult == _details::ApplyDiffResult::CONFLICT)
    {
        rebaseFilesHelper.createAmendFile(commits.getHeadCommitHash());
        rebaseFilesHelper.createMessageFile(messageSquash);
        rebaseFilesHelper.appendCurrentFixupFile(rebaseTodoCommand);

        return RebaseResult::CONFLICT;
    }

    if (!isNextCommandFixupOrSquash())
    {
        rebaseFilesHelper.createAmendFile(commits.getHeadCommitHash());
        rebaseFilesHelper.createMessageFile(messageSquash);

        return RebaseResult::SQUASH;
    }

    rebaseFilesHelper.appendRewrittenPendingFile(rebaseTodoCommand.hash);
    rebaseFilesHelper.appendCurrentFixupFile(rebaseTodoCommand);

    const auto headCommitHash = commits.getHeadCommitHash();
    const auto headCommitInfo = commits.getCommitInfo(headCommitHash);
    const auto newCommitHash = amendCommit.amend(headCommitInfo, messageSquash);
    refs.updateRefHash("HEAD", newCommitHash);

    return RebaseResult::COMMAND_PROCESSED;
}

auto Rebase::pickCommit(const Commit& commitInfo) const -> std::expected<std::string, RebaseResult>
{
    const auto& pickedParent = commitInfo.getParents()[0];
    const auto headCommitHash = commits.getHeadCommitHash();

    if (headCommitHash == pickedParent)
    {
        // can FastForward
        branches.detachHead(commitInfo.getHash());

        return commitInfo.getHash();
    }

    const auto applyDiffResult = applyDiff.apply(commitInfo.getHash());

    if (applyDiffResult == _details::ApplyDiffResult::NO_CHANGES)
    {
        return std::unexpected{ RebaseResult::EMPTY_DIFF };
    }

    if (applyDiffResult == _details::ApplyDiffResult::CONFLICT)
    {
        return std::unexpected{ RebaseResult::CONFLICT };
    }

    const auto envp = std::vector<std::string>{
        "GIT_AUTHOR_NAME=" + commitInfo.getAuthor().name,
        "GIT_AUTHOR_EMAIL=" + commitInfo.getAuthor().email,
        "GIT_AUTHOR_DATE=" + commitInfo.getAuthorDate()
    };

    auto newCommitHash = createCommit.createCommit(commitInfo.getMessage(), commitInfo.getDescription(), { headCommitHash }, envp);
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

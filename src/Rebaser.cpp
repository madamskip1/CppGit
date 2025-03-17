#include "CppGit/Rebaser.hpp"

#include "CppGit/BranchesManager.hpp"
#include "CppGit/Commit.hpp"
#include "CppGit/CommitsLogManager.hpp"
#include "CppGit/CommitsManager.hpp"
#include "CppGit/IndexManager.hpp"
#include "CppGit/RebaseTodoCommand.hpp"
#include "CppGit/Repository.hpp"
#include "CppGit/_details/CommitAmender.hpp"
#include "CppGit/_details/CommitCreator.hpp"
#include "CppGit/_details/DiffApplier.hpp"
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
Rebaser::Rebaser(const Repository& repository)
    : repository{ &repository },
      commitsManager{ repository },
      branchesManager{ repository },
      referencesManager{ repository },
      indexWorktreeManager{ repository },
      rebaseFilesHelper{ repository },
      diffApplier{ repository },
      commitAmender{ repository },
      commitCreator{ repository }
{
}

auto Rebaser::rebase(const std::string_view upstream) const -> std::expected<std::string, RebaseResult>
{
    auto rebaseComamnds = getDefaultTodoCommands(upstream);
    return rebaseImpl(upstream, rebaseComamnds);
}

auto Rebaser::interactiveRebase(const std::string_view upstream, const std::vector<RebaseTodoCommand>& rebaseCommands) const -> std::expected<std::string, RebaseResult>
{
    return rebaseImpl(upstream, rebaseCommands);
}

auto Rebaser::abortRebase() const -> void
{
    indexWorktreeManager.resetIndexToTree(rebaseFilesHelper.getOrigHead());
    indexWorktreeManager.copyForceIndexToWorktree();
    referencesManager.updateSymbolicRef("HEAD", rebaseFilesHelper.getHeadName());

    rebaseFilesHelper.deleteAllRebaseFiles();
}

auto Rebaser::continueRebase() const -> std::expected<std::string, RebaseResult>
{
    return continueRebase("", "");
}

auto Rebaser::continueRebase(const std::string_view message, const std::string_view description) const -> std::expected<std::string, RebaseResult>
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
            hashAfter = commitsManager.getHeadCommitHash();

            if (lastCommand->type != RebaseTodoCommandType::EDIT || repository->IndexManager().areAnyStagedFiles())
            {
                const auto headCommitInfo = commitsManager.getCommitInfo(hashAfter);
                hashAfter = commitAmender.amendCommit(headCommitInfo, messageAndDesc);
                referencesManager.updateRefHash("HEAD", hashAfter);
            }

            rebaseFilesHelper.removeAmendFile();
        }
        else
        {
            const auto authorScript = rebaseFilesHelper.getAuthorScriptFile(); // authorScript has struct like envp
            const auto parent = commitsManager.getHeadCommitHash();

            hashAfter = commitCreator.createCommit(messageAndDesc, { parent }, authorScript);
            referencesManager.updateRefHash("HEAD", hashAfter);
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

auto Rebaser::isRebaseInProgress() const -> bool
{
    return std::filesystem::exists(repository->getGitDirectoryPath() / "rebase-merge" / "git-rebase-todo");
}

auto Rebaser::getDefaultTodoCommands(const std::string_view upstream) const -> std::vector<RebaseTodoCommand>
{
    auto rebaseBase = repository->executeGitCommand("merge-base", "HEAD", upstream);

    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CommitsLogManager::Order::REVERSE);
    auto commitsToRebase = commitsLogManager.getCommitsLogDetailed(std::move(rebaseBase.stdout), "HEAD");

    auto rebaseCommands = std::vector<RebaseTodoCommand>{};

    std::ranges::transform(commitsToRebase,
                           std::back_inserter(rebaseCommands),
                           [](auto& commit) {
                               return RebaseTodoCommand{ RebaseTodoCommandType::PICK, std::move(commit.getHash()), std::move(commit.getMessage()) };
                           });

    return rebaseCommands;
}

auto Rebaser::getStoppedMessage() const -> std::string
{
    return rebaseFilesHelper.getMessageFile();
}

auto Rebaser::rebaseImpl(const std::string_view upstream, const std::vector<RebaseTodoCommand>& rebaseCommands) const -> std::expected<std::string, RebaseResult>
{
    startRebase(upstream, rebaseCommands);

    if (auto todoResult = processTodoList(); todoResult != RebaseResult::COMMAND_PROCESSED)
    {
        return std::unexpected{ todoResult };
    }

    return endRebase();
}

auto Rebaser::startRebase(const std::string_view upstream, const std::vector<RebaseTodoCommand>& rebaseCommands) const -> void
{
    const auto upstreamHash = referencesManager.getRefHash(upstream);
    const auto headRef = referencesManager.getRefHash("HEAD");

    rebaseFilesHelper.createRebaseDir();
    rebaseFilesHelper.createHeadNameFile(branchesManager.getCurrentBranchName());
    rebaseFilesHelper.createOntoFile(upstreamHash);
    rebaseFilesHelper.createRebaseOrigHeadFile(headRef);
    _details::GitFilesHelper{ *repository }.setOrigHeadFile(headRef);
    rebaseFilesHelper.generateTodoFile(rebaseCommands);

    branchesManager.detachHead(upstreamHash);
}

auto Rebaser::endRebase() const -> std::string
{
    const auto currentHash = referencesManager.getRefHash("HEAD");
    const auto headName = rebaseFilesHelper.getHeadName();
    referencesManager.updateRefHash(headName, currentHash);
    referencesManager.updateSymbolicRef("HEAD", headName);

    rebaseFilesHelper.deleteAllRebaseFiles();

    return currentHash;
}

auto Rebaser::processTodoList() const -> RebaseResult
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

auto Rebaser::processTodoCommand(const RebaseTodoCommand& rebaseTodoCommand) const -> RebaseResult
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

auto Rebaser::processPickCommand(const RebaseTodoCommand& rebaseTodoCommand) const -> RebaseResult
{
    const auto commitInfo = commitsManager.getCommitInfo(rebaseTodoCommand.hash);
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

auto Rebaser::processBreakCommand(const RebaseTodoCommand& /*rebaseTodoCommand*/) -> RebaseResult
{
    return RebaseResult::BREAK;
}

auto Rebaser::processReword(const RebaseTodoCommand& rebaseTodoCommand) const -> RebaseResult
{
    const auto commitInfo = commitsManager.getCommitInfo(rebaseTodoCommand.hash);
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

auto Rebaser::processEdit(const RebaseTodoCommand& rebaseTodoCommand) const -> RebaseResult
{
    const auto commitInfo = commitsManager.getCommitInfo(rebaseTodoCommand.hash);
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

auto Rebaser::processDrop(const RebaseTodoCommand& /*rebaseTodoCommand*/) -> RebaseResult
{
    return RebaseResult::COMMAND_PROCESSED;
}

auto Rebaser::processFixup(const RebaseTodoCommand& rebaseTodoCommand) const -> RebaseResult
{
    const auto headCommitHash = commitsManager.getHeadCommitHash();
    const auto headCommitInfo = commitsManager.getCommitInfo(headCommitHash);

    if (auto applyResult = diffApplier.applyDiff(rebaseTodoCommand.hash); applyResult == _details::ApplyDiffResult::CONFLICT)
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

        const auto newCommitHash = commitAmender.amendCommit(headCommitInfo);
        referencesManager.updateRefHash("HEAD", newCommitHash);

        return RebaseResult::COMMAND_PROCESSED;
    }

    if (rebaseFilesHelper.areAnySquashInCurrentFixup())
    {
        rebaseFilesHelper.createAmendFile(headCommitHash);
        rebaseFilesHelper.appendCurrentFixupFile(rebaseTodoCommand);
        rebaseFilesHelper.createMessageFile(headCommitInfo.getMessageAndDescription());

        return RebaseResult::SQUASH;
    }

    const auto newCommitHash = commitAmender.amendCommit(headCommitInfo);
    referencesManager.updateRefHash("HEAD", newCommitHash);

    rebaseFilesHelper.appendRewrittenPendingFile(rebaseTodoCommand.hash);
    rebaseFilesHelper.appendRewrittenListWithRewrittenPending(newCommitHash);
    rebaseFilesHelper.removeCurrentFixupFile();

    return RebaseResult::COMMAND_PROCESSED;
}

auto Rebaser::processSquash(const RebaseTodoCommand& rebaseTodoCommand) const -> RebaseResult
{
    const auto messageSquash = getConcatenatedMessagePreviousAndCurrentCommit(commitsManager.getHeadCommitHash(), rebaseTodoCommand.hash);

    if (const auto applyResult = diffApplier.applyDiff(rebaseTodoCommand.hash); applyResult == _details::ApplyDiffResult::CONFLICT)
    {
        rebaseFilesHelper.createAmendFile(commitsManager.getHeadCommitHash());
        rebaseFilesHelper.createMessageFile(messageSquash);
        rebaseFilesHelper.appendCurrentFixupFile(rebaseTodoCommand);

        return RebaseResult::CONFLICT;
    }

    if (!isNextCommandFixupOrSquash())
    {
        rebaseFilesHelper.createAmendFile(commitsManager.getHeadCommitHash());
        rebaseFilesHelper.createMessageFile(messageSquash);

        return RebaseResult::SQUASH;
    }

    rebaseFilesHelper.appendRewrittenPendingFile(rebaseTodoCommand.hash);
    rebaseFilesHelper.appendCurrentFixupFile(rebaseTodoCommand);

    const auto headCommitHash = commitsManager.getHeadCommitHash();
    const auto headCommitInfo = commitsManager.getCommitInfo(headCommitHash);
    const auto newCommitHash = commitAmender.amendCommit(headCommitInfo, messageSquash);
    referencesManager.updateRefHash("HEAD", newCommitHash);

    return RebaseResult::COMMAND_PROCESSED;
}

auto Rebaser::pickCommit(const Commit& commitInfo) const -> std::expected<std::string, RebaseResult>
{
    const auto& pickedParent = commitInfo.getParents()[0];
    const auto headCommitHash = commitsManager.getHeadCommitHash();

    if (headCommitHash == pickedParent)
    {
        // can FastForward
        branchesManager.detachHead(commitInfo.getHash());

        return commitInfo.getHash();
    }

    const auto applyDiffResult = diffApplier.applyDiff(commitInfo.getHash());

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

    auto newCommitHash = commitCreator.createCommit(commitInfo.getMessage(), commitInfo.getDescription(), { headCommitHash }, envp);
    referencesManager.updateRefHash("HEAD", newCommitHash);

    return newCommitHash;
}

auto Rebaser::isNextCommandFixupOrSquash() const -> bool
{
    const auto peakCommand = rebaseFilesHelper.peekTodoFile();

    return peakCommand && (peakCommand->type == RebaseTodoCommandType::FIXUP || peakCommand->type == RebaseTodoCommandType::SQUASH);
}

auto Rebaser::getConcatenatedMessagePreviousAndCurrentCommit(const std::string_view previousCommitHash, const std::string_view currentCommitHash) const -> std::string
{
    const auto previousCommitInfo = commitsManager.getCommitInfo(previousCommitHash);
    const auto currentCommitInfo = commitsManager.getCommitInfo(currentCommitHash);

    return previousCommitInfo.getMessageAndDescription() + "\n\n" + currentCommitInfo.getMessageAndDescription();
}
} // namespace CppGit

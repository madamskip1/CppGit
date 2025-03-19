#include "CppGit/Merger.hpp"

#include "CppGit/BranchesManager.hpp"
#include "CppGit/CommitsManager.hpp"
#include "CppGit/IndexManager.hpp"
#include "CppGit/Repository.hpp"
#include "CppGit/Resetter.hpp"
#include "CppGit/_details/CommitCreator.hpp"
#include "CppGit/_details/FileUtility.hpp"
#include "CppGit/_details/GitFilesHelper.hpp"
#include "CppGit/_details/IndexWorktreeManager.hpp"

#include <expected>
#include <filesystem>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace CppGit {
Merger::Merger(const Repository& repository)
    : repository{ &repository },
      threeWayMerger{ repository },
      indexWorktreeManager{ repository },
      gitFilesHelper{ repository }
{
}


auto Merger::mergeFastForward(const std::string_view sourceBranch) const -> std::expected<std::string, MergeResult>
{
    return mergeFastForward(sourceBranch, "HEAD");
}

auto Merger::mergeFastForward(const std::string_view sourceBranch, const std::string_view targetBranch) const -> std::expected<std::string, MergeResult>
{
    const auto ancestor = getAncestor(sourceBranch, targetBranch);

    const auto branchesManager = repository->BranchesManager();

    auto sourceBranchRef = branchesManager.getHashBranchRefersTo(sourceBranch);
    auto targetBranchRef = branchesManager.getHashBranchRefersTo(targetBranch);

    if (ancestor == sourceBranchRef)
    {
        // Nothing to merge
        return targetBranchRef;
    }

    if (ancestor != targetBranchRef)
    {
        return std::unexpected{ MergeResult::FF_BRANCHES_DIVERGENCE };
    }

    gitFilesHelper.setOrigHeadFile(targetBranchRef);
    repository->Resetter().resetHard(sourceBranchRef);

    return sourceBranchRef;
}

auto Merger::mergeNoFastForward(const std::string_view sourceBranch, const std::string_view message, const std::string_view description) const -> std::expected<std::string, MergeResult>
{
    auto mergeBaseOutput = repository->executeGitCommand("merge-base", "HEAD", sourceBranch);

    auto& mergeBase = mergeBaseOutput.stdout;

    const auto branchesManager = repository->BranchesManager();
    auto sourceBranchRef = branchesManager.getHashBranchRefersTo(sourceBranch);
    auto targetBranchRef = branchesManager.getHashBranchRefersTo("HEAD");

    if (mergeBase == sourceBranchRef)
    {
        return std::unexpected{ MergeResult::NOTHING_TO_MERGE };
    }

    repository->executeGitCommand("read-tree", "-m", std::move(mergeBase), "HEAD", sourceBranch);

    gitFilesHelper.setOrigHeadFile(targetBranchRef);
    indexWorktreeManager.copyIndexToWorktree();


    if (auto unmergedFilesEntries = repository->IndexManager().getUnmergedFilesDetailedList(); !unmergedFilesEntries.empty())
    {
        startMergeConflict(unmergedFilesEntries, std::move(sourceBranchRef), sourceBranch, "HEAD", message, description);

        return std::unexpected{ MergeResult::NO_FF_CONFLICT };
    }

    return createMergeCommit(std::move(sourceBranchRef), std::move(targetBranchRef), message, description);
}


auto Merger::canFastForward(const std::string_view sourceBranch) const -> bool
{
    return canFastForward(sourceBranch, "HEAD");
}

auto Merger::canFastForward(const std::string_view sourceBranch, const std::string_view targetBranch) const -> bool
{
    const auto ancestor = getAncestor(sourceBranch, targetBranch);
    return ancestor == repository->CommitsManager().getHeadCommitHash();
}

auto Merger::isAnythingToMerge(const std::string_view sourceBranch) const -> bool
{
    return isAnythingToMerge(sourceBranch, "HEAD");
}

auto Merger::isAnythingToMerge(const std::string_view sourceBranch, const std::string_view targetBranch) const -> bool
{
    const auto ancestor = getAncestor(sourceBranch, targetBranch);
    const auto sourceBranchRef = repository->BranchesManager().getHashBranchRefersTo(sourceBranch);

    return ancestor != sourceBranchRef;
}

auto Merger::isMergeInProgress() const -> bool
{
    const auto topLevelPath = repository->getTopLevelPath();
    return std::filesystem::exists(topLevelPath / ".git/MERGE_HEAD");
}

auto Merger::isThereAnyConflict() const -> bool
{
    return isThereAnyConflictImpl();
}

auto Merger::getAncestor(const std::string_view sourceBranch, const std::string_view targetBranch) const -> std::string
{
    auto output = repository->executeGitCommand("merge-base", sourceBranch, targetBranch);
    return std::move(output.stdout);
}

auto Merger::createMergeCommit(const std::string_view sourceBranchRef, const std::string_view targetBranchRef, const std::string_view message, const std::string_view description) const -> std::string
{
    const auto parents = std::vector<std::string>{ std::string{ targetBranchRef }, std::string{ sourceBranchRef } };

    const auto mergeCommitHash = _details::CommitCreator{ *repository }.createCommit(message, description, parents, {});
    _details::ReferencesManager{ *repository }.updateRefHash("HEAD", mergeCommitHash);

    return mergeCommitHash;
}

auto Merger::startMergeConflict(const std::vector<IndexEntry>& unmergedFilesEntries, const std::string_view sourceBranchRef, const std::string_view sourceLabel, const std::string_view targetLabel, const std::string_view message, const std::string_view description) const -> void
{
    createNoFFMergeFiles(sourceBranchRef, message, description);
    threeWayMerger.mergeConflictedFiles(unmergedFilesEntries, sourceLabel, targetLabel);
}

auto Merger::createNoFFMergeFiles(const std::string_view sourceBranchRef, const std::string_view message, const std::string_view description) const -> void
{
    const auto topLevelPath = repository->getTopLevelPath();
    _details::FileUtility::createOrOverwriteFile(topLevelPath / ".git/MERGE_HEAD", sourceBranchRef);
    _details::FileUtility::createOrOverwriteFile(topLevelPath / ".git/MERGE_MODE", "no-ff");
    threeWayMerger.createMergeMsgFile(message, description);
}

auto Merger::removeNoFFMergeFiles() const -> void
{
    const auto topLevelPath = repository->getTopLevelPath();
    std::filesystem::remove(topLevelPath / ".git/MERGE_HEAD");
    std::filesystem::remove(topLevelPath / ".git/MERGE_MODE");
    threeWayMerger.removeMergeMsgFile();
}

auto Merger::isThereAnyConflictImpl() const -> bool
{
    const auto gitLsFilesOutput = repository->executeGitCommand("ls-files", "-u");
    return !gitLsFilesOutput.stdout.empty();
}

auto Merger::abortMerge() const -> void
{
    indexWorktreeManager.resetIndexToTree("HEAD");
    removeNoFFMergeFiles();
}

auto Merger::continueMerge() const -> std::expected<std::string, MergeResult>
{
    if (isThereAnyConflictImpl())
    {
        return std::unexpected{ MergeResult::NO_FF_CONFLICT };
    }

    const auto mergeMsg = threeWayMerger.getMergeMsg();
    const auto mergeHead = _details::FileUtility::readFile(repository->getTopLevelPath() / ".git/MERGE_HEAD");
    const auto headCommitHash = repository->CommitsManager().getHeadCommitHash();

    auto mergeCommitHash = createMergeCommit(mergeHead, headCommitHash, mergeMsg, "");

    removeNoFFMergeFiles();

    return mergeCommitHash;
}

} // namespace CppGit

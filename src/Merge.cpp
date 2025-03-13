#include "CppGit/Merge.hpp"

#include "CppGit/Branches.hpp"
#include "CppGit/Commits.hpp"
#include "CppGit/Index.hpp"
#include "CppGit/Repository.hpp"
#include "CppGit/Reset.hpp"
#include "CppGit/_details/CreateCommit.hpp"
#include "CppGit/_details/FileUtility.hpp"
#include "CppGit/_details/GitFilesHelper.hpp"
#include "CppGit/_details/IndexWorktree.hpp"

#include <expected>
#include <filesystem>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace CppGit {
Merge::Merge(const Repository& repo)
    : repo{ &repo },
      threeWayMerge{ repo },
      indexWorktree{ repo },
      gitFilesHelper{ repo }
{
}


auto Merge::mergeFastForward(const std::string_view sourceBranch) const -> std::expected<std::string, MergeResult>
{
    return mergeFastForward(sourceBranch, "HEAD");
}

auto Merge::mergeFastForward(const std::string_view sourceBranch, const std::string_view targetBranch) const -> std::expected<std::string, MergeResult>
{
    const auto ancestor = getAncestor(sourceBranch, targetBranch);

    const auto branches = repo->Branches();

    auto sourceBranchRef = branches.getHashBranchRefersTo(sourceBranch);
    auto targetBranchRef = branches.getHashBranchRefersTo(targetBranch);

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
    repo->Reset().resetHard(sourceBranchRef);

    return sourceBranchRef;
}

auto Merge::mergeNoFastForward(const std::string_view sourceBranch, const std::string_view message, const std::string_view description) const -> std::expected<std::string, MergeResult>
{
    const auto index = repo->Index();

    auto mergeBaseOutput = repo->executeGitCommand("merge-base", "HEAD", sourceBranch);

    auto& mergeBase = mergeBaseOutput.stdout;

    const auto branches = repo->Branches();
    auto sourceBranchRef = branches.getHashBranchRefersTo(sourceBranch);
    auto targetBranchRef = branches.getHashBranchRefersTo("HEAD");

    if (mergeBase == sourceBranchRef)
    {
        return std::unexpected{ MergeResult::NOTHING_TO_MERGE };
    }

    repo->executeGitCommand("read-tree", "-m", std::move(mergeBase), "HEAD", sourceBranch);

    gitFilesHelper.setOrigHeadFile(targetBranchRef);
    indexWorktree.copyIndexToWorktree();


    if (auto unmergedFilesEntries = index.getUnmergedFilesListWithDetails(); !unmergedFilesEntries.empty())
    {
        startMergeConflict(unmergedFilesEntries, std::move(sourceBranchRef), sourceBranch, "HEAD", message, description);

        return std::unexpected{ MergeResult::NO_FF_CONFLICT };
    }

    return createMergeCommit(std::move(sourceBranchRef), std::move(targetBranchRef), message, description);
}


auto Merge::canFastForward(const std::string_view sourceBranch) const -> bool
{
    return canFastForward(sourceBranch, "HEAD");
}

auto Merge::canFastForward(const std::string_view sourceBranch, const std::string_view targetBranch) const -> bool
{
    const auto ancestor = getAncestor(sourceBranch, targetBranch);
    const auto commits = repo->Commits();

    return ancestor == commits.getHeadCommitHash();
}

auto Merge::isAnythingToMerge(const std::string_view sourceBranch) const -> bool
{
    return isAnythingToMerge(sourceBranch, "HEAD");
}

auto Merge::isAnythingToMerge(const std::string_view sourceBranch, const std::string_view targetBranch) const -> bool
{
    const auto ancestor = getAncestor(sourceBranch, targetBranch);
    const auto branches = repo->Branches();
    const auto sourceBranchRef = branches.getHashBranchRefersTo(sourceBranch);

    return ancestor != sourceBranchRef;
}

auto Merge::isMergeInProgress() const -> bool
{
    const auto topLevelPath = repo->getTopLevelPath();
    return std::filesystem::exists(topLevelPath / ".git/MERGE_HEAD");
}

auto Merge::isThereAnyConflict() const -> bool
{
    return isThereAnyConflictImpl();
}

auto Merge::getAncestor(const std::string_view sourceBranch, const std::string_view targetBranch) const -> std::string
{
    auto output = repo->executeGitCommand("merge-base", sourceBranch, targetBranch);
    return std::move(output.stdout);
}

auto Merge::createMergeCommit(const std::string_view sourceBranchRef, const std::string_view targetBranchRef, const std::string_view message, const std::string_view description) const -> std::string
{
    const auto parents = std::vector<std::string>{ std::string{ targetBranchRef }, std::string{ sourceBranchRef } };

    const auto mergeCommitHash = _details::CreateCommit{ *repo }.createCommit(message, description, parents, {});
    _details::Refs{ *repo }.updateRefHash("HEAD", mergeCommitHash);

    return mergeCommitHash;
}

auto Merge::startMergeConflict(const std::vector<IndexEntry>& unmergedFilesEntries, const std::string_view sourceBranchRef, const std::string_view sourceLabel, const std::string_view targetLabel, const std::string_view message, const std::string_view description) const -> void
{
    createNoFFMergeFiles(sourceBranchRef, message, description);
    threeWayMerge.mergeConflictedFiles(unmergedFilesEntries, sourceLabel, targetLabel);
}

auto Merge::createNoFFMergeFiles(const std::string_view sourceBranchRef, const std::string_view message, const std::string_view description) const -> void
{
    const auto topLevelPath = repo->getTopLevelPath();
    _details::FileUtility::createOrOverwriteFile(topLevelPath / ".git/MERGE_HEAD", sourceBranchRef);
    _details::FileUtility::createOrOverwriteFile(topLevelPath / ".git/MERGE_MODE", "no-ff");
    threeWayMerge.createMergeMsgFile(message, description);
}

auto Merge::removeNoFFMergeFiles() const -> void
{
    const auto topLevelPath = repo->getTopLevelPath();
    std::filesystem::remove(topLevelPath / ".git/MERGE_HEAD");
    std::filesystem::remove(topLevelPath / ".git/MERGE_MODE");
    threeWayMerge.removeMergeMsgFile();
}

auto Merge::isThereAnyConflictImpl() const -> bool
{
    const auto gitLsFilesOutput = repo->executeGitCommand("ls-files", "-u");
    return !gitLsFilesOutput.stdout.empty();
}

auto Merge::abortMerge() const -> void
{
    indexWorktree.resetIndexToTree("HEAD");
    removeNoFFMergeFiles();
}

auto Merge::continueMerge() const -> std::expected<std::string, MergeResult>
{
    if (isThereAnyConflictImpl())
    {
        return std::unexpected{ MergeResult::NO_FF_CONFLICT };
    }

    const auto mergeMsg = threeWayMerge.getMergeMsg();
    const auto mergeHead = _details::FileUtility::readFile(repo->getTopLevelPath() / ".git/MERGE_HEAD");
    const auto headCommitHash = repo->Commits().getHeadCommitHash();

    auto mergeCommitHash = createMergeCommit(mergeHead, headCommitHash, mergeMsg, "");

    removeNoFFMergeFiles();

    return mergeCommitHash;
}

} // namespace CppGit

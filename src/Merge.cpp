#include "CppGit/Merge.hpp"

#include "CppGit/Branches.hpp"
#include "CppGit/Commits.hpp"
#include "CppGit/Error.hpp"
#include "CppGit/Index.hpp"
#include "CppGit/Repository.hpp"
#include "CppGit/Reset.hpp"
#include "CppGit/_details/CreateCommit.hpp"
#include "CppGit/_details/FileUtility.hpp"
#include "CppGit/_details/GitCommandExecutor/GitCommandOutput.hpp"
#include "CppGit/_details/GitFilesHelper.hpp"
#include "CppGit/_details/IndexWorktree.hpp"

#include <expected>
#include <filesystem>
#include <stdexcept>
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


auto Merge::mergeFastForward(const std::string_view sourceBranch) const -> std::expected<std::string, Error>
{
    return mergeFastForward(sourceBranch, "HEAD");
}

auto Merge::mergeFastForward(const std::string_view sourceBranch, const std::string_view targetBranch) const -> std::expected<std::string, Error>
{
    if (const auto index = repo->Index(); index.isDirty())
    {
        return std::unexpected{ Error::DIRTY_WORKTREE };
    }

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
        return std::unexpected{ Error::MERGE_FF_BRANCHES_DIVERGENCE };
    }

    gitFilesHelper.setOrigHeadFile(targetBranchRef);
    Reset{ *repo }.resetHard(sourceBranchRef);

    return sourceBranchRef;
}

auto Merge::mergeNoFastForward(const std::string_view sourceBranch, const std::string_view message, const std::string_view description) const -> std::expected<std::string, Error>
{
    const auto index = repo->Index();

    if (index.isDirty())
    {
        return std::unexpected{ Error::DIRTY_WORKTREE };
    }

    auto mergeBaseOutput = repo->executeGitCommand("merge-base", "HEAD", sourceBranch);
    if (mergeBaseOutput.return_code != 0)
    {
        throw std::runtime_error("Failed to find merge base");
    }

    auto& mergeBase = mergeBaseOutput.stdout;

    const auto branches = repo->Branches();
    auto sourceBranchRef = branches.getHashBranchRefersTo(sourceBranch);
    auto targetBranchRef = branches.getHashBranchRefersTo("HEAD");

    if (mergeBase == sourceBranchRef)
    {
        return std::unexpected{ Error::MERGE_NOTHING_TO_MERGE };
    }

    auto readTreeOutput = repo->executeGitCommand("read-tree", "-m", std::move(mergeBase), "HEAD", sourceBranch);

    if (readTreeOutput.return_code != 0)
    {
        throw std::runtime_error("Failed to read tree");
    }

    gitFilesHelper.setOrigHeadFile(targetBranchRef);

    indexWorktree.copyIndexToWorktree();


    if (auto unmergedFilesEntries = index.getUnmergedFilesListWithDetails(); !unmergedFilesEntries.empty())
    {
        startMergeConflict(unmergedFilesEntries, std::move(sourceBranchRef), sourceBranch, "HEAD", message, description);

        return std::unexpected{ Error::MERGE_NO_FF_CONFLICT };
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

auto Merge::isThereAnyConflict() const -> std::expected<bool, Error>
{
    if (!isMergeInProgress())
    {
        return std::unexpected{ Error::NO_MERGE_IN_PROGRESS };
    }

    return isThereAnyConflictImpl();
}

auto Merge::getAncestor(const std::string_view sourceBranch, const std::string_view targetBranch) const -> std::string
{
    auto output = repo->executeGitCommand("merge-base", sourceBranch, targetBranch);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to find merge base");
    }

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

    if (gitLsFilesOutput.return_code != 0)
    {
        throw std::runtime_error("Failed to list files");
    }

    return !gitLsFilesOutput.stdout.empty();
}

auto Merge::abortMerge() const -> Error
{
    if (!isMergeInProgress())
    {
        return Error::NO_MERGE_IN_PROGRESS;
    }

    indexWorktree.resetIndexToTree("HEAD");
    removeNoFFMergeFiles();

    return Error::NO_ERROR;
}

auto Merge::continueMerge() const -> std::expected<std::string, Error>
{
    if (!isMergeInProgress())
    {
        return std::unexpected{ Error::NO_MERGE_IN_PROGRESS };
    }

    if (isThereAnyConflictImpl())
    {
        return std::unexpected{ Error::MERGE_NO_FF_CONFLICT };
    }

    const auto mergeMsg = threeWayMerge.getMergeMsg();
    const auto mergeHead = _details::FileUtility::readFile(repo->getTopLevelPath() / ".git/MERGE_HEAD");
    const auto headCommitHash = repo->Commits().getHeadCommitHash();

    auto mergeCommitHash = createMergeCommit(mergeHead, headCommitHash, mergeMsg, "");

    removeNoFFMergeFiles();

    return mergeCommitHash;
}

} // namespace CppGit

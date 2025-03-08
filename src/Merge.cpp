#include "Merge.hpp"

#include "Branches.hpp"
#include "Commits.hpp"
#include "Error.hpp"
#include "Index.hpp"
#include "Repository.hpp"
#include "Reset.hpp"
#include "_details/FileUtility.hpp"
#include "_details/GitCommandExecutor/GitCommandOutput.hpp"
#include "_details/GitFilesHelper.hpp"

#include <expected>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace CppGit {
Merge::Merge(const Repository& repo)
    : repo(repo),
      _createCommit(repo),
      _threeWayMerge(repo),
      _indexWorktree(repo)
{
}


auto Merge::mergeFastForward(const std::string_view sourceBranch) const -> std::expected<std::string, Error>
{
    return mergeFastForward(sourceBranch, "HEAD");
}

auto Merge::mergeFastForward(const std::string_view sourceBranch, const std::string_view targetBranch) const -> std::expected<std::string, Error>
{
    if (const auto index = repo.Index(); index.isDirty())
    {
        return std::unexpected{ Error::DIRTY_WORKTREE };
    }

    const auto ancestor = getAncestor(sourceBranch, targetBranch);

    const auto branches = repo.Branches();

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

    _details::GitFilesHelper{ repo }.setOrigHeadFile(targetBranchRef);
    Reset{ repo }.resetHard(sourceBranchRef);

    return sourceBranchRef;
}

auto Merge::mergeNoFastForward(const std::string_view sourceBranch, const std::string_view message, const std::string_view description) -> std::expected<std::string, Error>
{
    const auto index = repo.Index();

    if (index.isDirty())
    {
        return std::unexpected{ Error::DIRTY_WORKTREE };
    }

    auto mergeBaseOutput = repo.executeGitCommand("merge-base", "HEAD", sourceBranch);
    if (mergeBaseOutput.return_code != 0)
    {
        throw std::runtime_error("Failed to find merge base");
    }

    auto& mergeBase = mergeBaseOutput.stdout;

    const auto branches = repo.Branches();
    auto sourceBranchRef = branches.getHashBranchRefersTo(sourceBranch);
    auto targetBranchRef = branches.getHashBranchRefersTo("HEAD");

    if (mergeBase == sourceBranchRef)
    {
        return std::unexpected{ Error::MERGE_NOTHING_TO_MERGE };
    }

    auto readTreeOutput = repo.executeGitCommand("read-tree", "-m", std::move(mergeBase), "HEAD", sourceBranch);

    if (readTreeOutput.return_code != 0)
    {
        throw std::runtime_error("Failed to read tree");
    }

    _details::GitFilesHelper{ repo }.setOrigHeadFile(targetBranchRef);

    _indexWorktree.copyIndexToWorktree();


    if (auto unmergedFilesEntries = index.getUnmergedFilesListWithDetails(); !unmergedFilesEntries.empty())
    {
        startMergeConflict(unmergedFilesEntries, std::move(sourceBranchRef), sourceBranch, std::move(targetBranchRef), "HEAD", message, description);

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
    const auto commits = repo.Commits();

    return ancestor == commits.getHeadCommitHash();
}

auto Merge::isAnythingToMerge(const std::string_view sourceBranch) const -> bool
{
    return isAnythingToMerge(sourceBranch, "HEAD");
}

auto Merge::isAnythingToMerge(const std::string_view sourceBranch, const std::string_view targetBranch) const -> bool
{
    const auto ancestor = getAncestor(sourceBranch, targetBranch);
    const auto branches = repo.Branches();
    const auto sourceBranchRef = branches.getHashBranchRefersTo(sourceBranch);

    return ancestor != sourceBranchRef;
}

auto Merge::isMergeInProgress() const -> bool
{
    const auto topLevelPath = repo.getTopLevelPath();
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
    auto output = repo.executeGitCommand("merge-base", sourceBranch, targetBranch);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to find merge base");
    }

    return std::move(output.stdout);
}

auto Merge::createMergeCommit(const std::string_view sourceBranchRef, const std::string_view targetBranchRef, const std::string_view message, const std::string_view description) const -> std::string
{
    const auto parents = std::vector<std::string>{ std::string{ targetBranchRef }, std::string{ sourceBranchRef } };

    const auto mergeCommitHash = _createCommit.createCommit(message, description, parents, {});
    _details::Refs{ repo }.updateRefHash("HEAD", mergeCommitHash);

    return mergeCommitHash;
}

auto Merge::startMergeConflict(const std::vector<IndexEntry>& unmergedFilesEntries, const std::string_view sourceBranchRef, const std::string_view sourceLabel, const std::string_view targetBranchRef, const std::string_view targetLabel, const std::string_view message, const std::string_view description) -> void
{
    createNoFFMergeFiles(sourceBranchRef, message, description);
    mergeInProgress_sourceBranchRef = std::string{ sourceBranchRef.cbegin(), sourceBranchRef.cend() };
    mergeInProgress_targetBranchRef = std::string{ targetBranchRef };

    _threeWayMerge.mergeConflictedFiles(unmergedFilesEntries, sourceLabel, targetLabel);
}

auto Merge::createNoFFMergeFiles(const std::string_view sourceBranchRef, const std::string_view message, const std::string_view description) const -> void
{
    const auto topLevelPath = repo.getTopLevelPath();
    _details::FileUtility::createOrOverwriteFile(topLevelPath / ".git/MERGE_HEAD", sourceBranchRef);
    _details::FileUtility::createOrOverwriteFile(topLevelPath / ".git/MERGE_MODE", "no-ff");
    _threeWayMerge.createMergeMsgFile(message, description);
}

auto Merge::removeNoFFMergeFiles() const -> void
{
    const auto topLevelPath = repo.getTopLevelPath();
    std::filesystem::remove(topLevelPath / ".git/MERGE_HEAD");
    std::filesystem::remove(topLevelPath / ".git/MERGE_MODE");
    _threeWayMerge.removeMergeMsgFile();
}

auto Merge::isThereAnyConflictImpl() const -> bool
{
    const auto gitLsFilesOutput = repo.executeGitCommand("ls-files", "-u");

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

    _indexWorktree.resetIndexToTree("HEAD");
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

    const auto mergeMsg = _threeWayMerge.getMergeMsg();

    auto mergeCommitHash = createMergeCommit(mergeInProgress_sourceBranchRef, mergeInProgress_targetBranchRef, mergeMsg, "");

    removeNoFFMergeFiles();

    return mergeCommitHash;
}

} // namespace CppGit

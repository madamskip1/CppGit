#include "Merge.hpp"

#include "Branches.hpp"
#include "Commits.hpp"
#include "Repository.hpp"
#include "_details/GitCommandExecutor/GitCommandOutput.hpp"

#include <filesystem>
#include <fstream>

namespace CppGit {
Merge::Merge(const Repository& repo)
    : repo(repo),
      _createCommit(repo)
{
}


auto Merge::mergeFastForward(const std::string_view sourceBranch) const -> std::string
{
    return mergeFastForward(sourceBranch, "HEAD");
}

auto Merge::mergeFastForward(const std::string_view sourceBranch, const std::string_view targetBranch) const -> std::string
{
    auto index = repo.Index();

    if (index.isDirty())
    {
        throw std::runtime_error("Cannot merge with dirty worktree");
    }

    auto ancestor = getAncestor(sourceBranch, targetBranch);

    auto branches = repo.Branches();

    auto sourceBranchRef = branches.getHashBranchRefersTo(sourceBranch);
    auto targetBranchRef = branches.getHashBranchRefersTo(targetBranch);

    if (ancestor == sourceBranchRef)
    {
        // Nothing to merge
        return targetBranchRef;
    }

    if (ancestor != targetBranchRef)
    {
        throw std::runtime_error("Cannot fast-forward");
    }

    branches.changeCurrentBranchRef(sourceBranchRef);

    return sourceBranchRef;
}

auto Merge::mergeNoFastForward(const std::string_view sourceBranch, const std::string_view message, const std::string_view description) -> std::string
{
    auto index = repo.Index();

    if (index.isDirty())
    {
        throw std::runtime_error("Cannot merge with dirty worktree");
    }

    auto mergeBaseOutput = repo.executeGitCommand("merge-base", "HEAD", sourceBranch);
    if (mergeBaseOutput.return_code != 0)
    {
        throw std::runtime_error("Failed to find merge base");
    }

    auto& mergeBase = mergeBaseOutput.stdout;

    auto branches = repo.Branches();
    auto sourceBranchRef = branches.getHashBranchRefersTo(sourceBranch);
    auto targetBranchRef = branches.getHashBranchRefersTo("HEAD");

    if (mergeBase == sourceBranchRef)
    {
        throw std::runtime_error("Nothing to merge");
    }

    if (mergeBase == sourceBranchRef)
    {
        throw std::runtime_error("Nothing to merge");
    }

    auto readTreeOutput = repo.executeGitCommand("read-tree", "-m", std::move(mergeBase), "HEAD", sourceBranch);

    if (readTreeOutput.return_code != 0)
    {
        throw std::runtime_error("Failed to read tree");
    }

    auto checkoutIndexOutput = repo.executeGitCommand("checkout-index", "-a");

    if (checkoutIndexOutput.return_code != 0)
    {
        throw std::runtime_error("Failed to checkout index");
    }

    auto unmergedFilesEntries = index.getUnmergedFilesListWithDetails();

    if (!unmergedFilesEntries.empty())
    {
        startMergeConflict(unmergedFilesEntries, std::move(sourceBranchRef), sourceBranch, std::move(targetBranchRef), "HEAD", message, description);
        throw std::runtime_error("Conflicts detected");
    }


    return createMergeCommit(std::move(sourceBranchRef), std::move(targetBranchRef), message, description);
}


auto Merge::canFastForward(const std::string_view sourceBranch) const -> bool
{
    return canFastForward(sourceBranch, "HEAD");
}

auto Merge::canFastForward(const std::string_view sourceBranch, const std::string_view targetBranch) const -> bool
{
    auto ancestor = getAncestor(sourceBranch, targetBranch);
    auto commits = repo.Commits();

    return ancestor == commits.getHeadCommitHash();
}

auto Merge::isAnythingToMerge(const std::string_view sourceBranch) const -> bool
{
    return isAnythingToMerge(sourceBranch, "HEAD");
}

auto Merge::isAnythingToMerge(const std::string_view sourceBranch, const std::string_view targetBranch) const -> bool
{
    auto ancestor = getAncestor(sourceBranch, targetBranch);
    auto branches = repo.Branches();
    auto sourceBranchRef = branches.getHashBranchRefersTo(sourceBranch);

    return ancestor != sourceBranchRef;
}

auto Merge::isMergeInProgress() const -> bool
{
    auto topLevelPath = repo.getTopLevelPath();
    return std::filesystem::exists(topLevelPath / ".git/MERGE_HEAD");
}

auto Merge::isThereAnyConflict() const -> bool
{
    auto gitLsFilesOutput = repo.executeGitCommand("ls-files", "-u");

    if (gitLsFilesOutput.return_code != 0)
    {
        throw std::runtime_error("Failed to list files");
    }


    return !gitLsFilesOutput.stdout.empty();
}

auto Merge::getAncestor(const std::string_view sourceBranch, const std::string_view targetBranch) const -> std::string
{
    auto output = repo.executeGitCommand("merge-base", sourceBranch, targetBranch);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to find merge base");
    }

    return output.stdout;
}

auto Merge::createMergeCommit(const std::string_view sourceBranchRef, const std::string_view targetBranchRef, const std::string_view message, const std::string_view description) const -> std::string
{
    auto parents = std::vector<std::string>{ std::string{ targetBranchRef }, std::string{ sourceBranchRef } };

    return _createCommit.createCommit(message, description, parents, {});
}

auto Merge::startMergeConflict(const std::vector<IndexEntry>& unmergedFilesEntries, const std::string_view sourceBranchRef, const std::string_view sourceLabel, const std::string_view targetBranchRef, const std::string_view targetLabel, const std::string_view message, const std::string_view description) -> void
{
    createNoFFMergeFiles(sourceBranchRef, message, description);
    mergeInProgress_sourceBranchRef = std::string{ sourceBranchRef.cbegin(), sourceBranchRef.cend() };
    mergeInProgress_targetBranchRef = std::string{ targetBranchRef };
    mergeInProgress_message = std::string{ message };
    mergeInProgress_description = std::string{ description };

    threeWayMergeConflictedFiles(unmergedFilesEntries, sourceLabel, targetLabel);
}

auto Merge::unpackFile(const std::string_view fileBlob) const -> std::string
{
    auto output = repo.executeGitCommand("unpack-file", fileBlob);

    return std::move(output.stdout);
}

auto Merge::parseUnmergedFiles(const std::vector<CppGit::IndexEntry>& indexEntries) const -> std::unordered_map<std::string, UnmergedFileBlobs>
{
    std::unordered_map<std::string, UnmergedFileBlobs> unmergedFiles;

    for (const auto& indexEntry : indexEntries)
    {
        auto& unmergedFile = unmergedFiles[indexEntry.path];
        const auto& fileBlob = indexEntry.objectHash;

        if (indexEntry.stageNumber == 1)
        {
            unmergedFile.baseBlob = fileBlob;
        }
        else if (indexEntry.stageNumber == 2)
        {
            unmergedFile.targetBlob = fileBlob;
        }
        else if (indexEntry.stageNumber == 3)
        {
            unmergedFile.sourceBlob = fileBlob;
        }
    }

    return unmergedFiles;
}

auto Merge::createNoFFMergeFiles(const std::string_view sourceBranchRef, const std::string_view message, const std::string_view description) const -> void
{
    auto topLevelPath = repo.getTopLevelPath();
    std::ofstream MERGE_HEAD{ topLevelPath / ".git/MERGE_HEAD" };
    MERGE_HEAD << sourceBranchRef;
    MERGE_HEAD.close();

    std::ofstream MERGE_MSG{ topLevelPath / ".git/MERGE_MSG" };
    MERGE_MSG << message;
    if (!description.empty())
    {
        MERGE_MSG << "\n\n"
                  << description;
    }
    MERGE_MSG.close();

    std::ofstream MERGE_MODE{ topLevelPath / ".git/MERGE_MODE" };
    MERGE_MODE << "no-ff";
    MERGE_MODE.close();
}

auto Merge::removeNoFFMergeFiles() const -> void
{
    auto topLevelPath = repo.getTopLevelPath();
    std::filesystem::remove(topLevelPath / ".git/MERGE_HEAD");
    std::filesystem::remove(topLevelPath / ".git/MERGE_MSG");
    std::filesystem::remove(topLevelPath / ".git/MERGE_MODE");
}

auto Merge::threeWayMergeConflictedFiles(const std::vector<IndexEntry>& unmergedFilesEntries, const std::string_view sourceLabel, const std::string_view targetLabel) const -> void
{
    const auto unmergedFiles = parseUnmergedFiles(unmergedFilesEntries);

    auto repoRootPath = repo.getTopLevelPath();

    for (const auto& [file, unmergedFile] : unmergedFiles)
    {
        auto baseTempFile = unpackFile(unmergedFile.baseBlob);
        auto targetTempFile = unpackFile(unmergedFile.targetBlob);
        auto sourceTempFile = unpackFile(unmergedFile.sourceBlob);

        if (baseTempFile.empty())
        {
            baseTempFile = "/dev/null";
        }

        auto mergeFileOutput = repo.executeGitCommand("merge-file", "-L", targetLabel, "-L", "ancestor", "-L", sourceLabel, targetTempFile, baseTempFile, sourceTempFile);

        auto checkoutIndexOutput = repo.executeGitCommand("checkout-index", "-f", "--stage=2", "--", file);

        auto baseTempFilePath = std::filesystem::path{ repoRootPath / baseTempFile };

        auto targetTempFilePath = std::filesystem::path{ repoRootPath / targetTempFile };
        auto sourceTempFilePath = std::filesystem::path{ repoRootPath / sourceTempFile };
        auto filePath = std::filesystem::path{ repoRootPath / file };

        auto src_file = std::ifstream{ targetTempFilePath, std::ios::binary };
        auto dst_file = std::ofstream{ filePath, std::ios::binary | std::ios::trunc };
        dst_file << src_file.rdbuf();
        src_file.close();
        dst_file.close();

        std::filesystem::remove(targetTempFilePath);
        std::filesystem::remove(sourceTempFilePath);
        if (baseTempFile != "/dev/null")
        {
            std::filesystem::remove(baseTempFilePath);
        }
    }
}

auto Merge::abortMerge() const -> void
{
    auto output = repo.executeGitCommand("read-tree", "--reset", "-u", "HEAD");

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to reset tree");
    }
    removeNoFFMergeFiles();
}

auto Merge::continueMerge() const -> std::string
{
    if (!isMergeInProgress())
    {
        throw std::runtime_error("No merge in progress");
    }

    if (isThereAnyConflict())
    {
        throw std::runtime_error("Cannot continue merge with conflicts");
    }

    auto mergeCommitHash = createMergeCommit(mergeInProgress_sourceBranchRef, mergeInProgress_targetBranchRef, mergeInProgress_message, mergeInProgress_description);

    removeNoFFMergeFiles();

    return mergeCommitHash;
}

} // namespace CppGit

#include "CherryPick.hpp"

#include "Commits.hpp"
#include "Exceptions.hpp"
#include "Index.hpp"
#include "_details/FileUtility.hpp"

#include <filesystem>

namespace CppGit {

CherryPick::CherryPick(const Repository& repo)
    : repo(repo),
      _createCommit(repo),
      _threeWayMerge(repo),
      _indexWorktree(repo)
{
}

auto CherryPick::cherryPickCommit(const std::string_view commitHash, CherryPickEmptyCommitStrategy emptyCommitStrategy) const -> std::string
{
    auto diffOutput = repo.executeGitCommand("diff-tree", "-p", commitHash);
    if (diffOutput.return_code != 0)
    {
        throw std::runtime_error("Failed to get diff");
    }

    if (diffOutput.stdout.empty())
    {
        return processEmptyDiff(commitHash, emptyCommitStrategy);
    }

    auto patchDifPath = repo.getGitDirectoryPath() / "patch.diff";
    _details::FileUtility::createOrOverwriteFile(patchDifPath, diffOutput.stdout);

    auto applyOutput = repo.executeGitCommand("apply", "--cached", "--3way", patchDifPath);

    std::filesystem::remove(patchDifPath);

    if (applyOutput.return_code != 0)
    {
        auto index = repo.Index();
        auto unmergedFilesEntries = index.getUnmergedFilesListWithDetails();

        if (!unmergedFilesEntries.empty())
        {
            _threeWayMerge.mergeConflictedFiles(unmergedFilesEntries, commitHash, "HEAD");

            createCherryPickHeadFile(commitHash);

            throw CppGit::MergeConflict{};
        }

        throw std::runtime_error("Failed to apply diff");
    }

    _indexWorktree.copyForceIndexToWorktree();

    auto index = repo.Index();

    // or maybe instead of getStagedFileList check write-tree hash and compare it with head commit tree hash
    if (index.getStagedFilesList().empty())
    {
        return processEmptyDiff(commitHash, emptyCommitStrategy);
    }

    if (applyOutput.return_code != 0)
    {
        throw std::runtime_error("Failed to apply diff");
    }

    return commitCherryPicked(commitHash);
}

auto CherryPick::commitEmptyCherryPickedCommit() const -> std::string
{
    auto commitHash = getCherryPickHead();
    if (commitHash.empty())
    {
        throw std::runtime_error("No cherry-picked commit");
    }

    auto commitedHash = commitCherryPicked(commitHash);
    std::filesystem::remove(repo.getGitDirectoryPath() / "CHERRY_PICK_HEAD");

    return commitedHash;
}

auto CherryPick::isCherryPickInProgress() const -> bool
{
    std::ifstream headFile(repo.getGitDirectoryPath() / "CHERRY_PICK_HEAD");
    if (headFile.is_open())
    {
        auto isEmpty = headFile.peek() == std::ifstream::traits_type::eof();
        headFile.close();

        return !isEmpty;
    }

    return false;
}

auto CherryPick::commitCherryPicked(const std::string_view commitHash) const -> std::string
{
    auto commits = Commits{ repo };

    auto commitInfo = commits.getCommitInfo(commitHash);

    auto envp = std::vector<std::string>{
        "GIT_AUTHOR_NAME=" + commitInfo.getAuthor().name,
        "GIT_AUTHOR_EMAIL=" + commitInfo.getAuthor().email,
        "GIT_AUTHOR_DATE=" + commitInfo.getAuthorDate()
    };

    auto parent = commits.hasAnyCommits() ? commits.getHeadCommitHash() : std::string{};

    return _createCommit.createCommit(commitInfo.getMessage(), commitInfo.getDescription(), { parent }, envp);
}

auto CherryPick::createCherryPickHeadFile(const std::string_view commitHash) const -> void
{
    _details::FileUtility::createOrOverwriteFile(repo.getGitDirectoryPath() / "CHERRY_PICK_HEAD", commitHash);
}

auto CherryPick::createConflictMsgFiles(const std::string_view message, const std::string_view description) const -> void
{
    _threeWayMerge.createMergeMsgFile(message, description);

    auto commitEditMsgFile = std::ofstream(repo.getGitDirectoryPath() / "COMMIT_EDITMSG");
    commitEditMsgFile << message;
    if (!description.empty())
    {
        commitEditMsgFile << "\n\n"
                          << description;
    }
    commitEditMsgFile.close();
}

auto CherryPick::getCherryPickHead() const -> std::string
{
    return _details::FileUtility::readFile(repo.getGitDirectoryPath() / "CHERRY_PICK_HEAD");
}

auto CherryPick::cherryPickContinue() const -> std::string
{
    if (!isCherryPickInProgress())
    {
        throw std::runtime_error("No cherry-pick in progress");
    }

    auto unmergedFilesEntries = repo.Index().getUnmergedFilesListWithDetails();

    if (!unmergedFilesEntries.empty())
    {
        throw std::runtime_error("Cannot continue cherry-pick with conflicts");
    }

    auto commitHash = getCherryPickHead();

    return commitCherryPicked(commitHash);
}

auto CppGit::CherryPick::processEmptyDiff(const std::string_view commitHash, CherryPickEmptyCommitStrategy emptyCommitStrategy) const -> std::string
{
    switch (emptyCommitStrategy)
    {
    case CherryPickEmptyCommitStrategy::DROP:
        return std::string(40, '0');
    case CherryPickEmptyCommitStrategy::KEEP:
        return commitCherryPicked(commitHash);
    case CherryPickEmptyCommitStrategy::STOP:
    default:
        createCherryPickHeadFile(commitHash);
        // createConflictMsgFiles("", ""); // TODO
        throw std::runtime_error("Empty commit");
    }
}

} // namespace CppGit

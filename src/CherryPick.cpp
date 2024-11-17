#include "CherryPick.hpp"

#include "Commits.hpp"
#include "Index.hpp"
#include "Merge.hpp"

#include <fstream>

namespace CppGit {

CherryPick::CherryPick(const Repository& repo)
    : repo(repo),
      _createCommit(repo)
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

    std::ofstream diffFile(repo.getGitDirectoryPath() / "patch.diff");
    diffFile << diffOutput.stdout;
    diffFile.close();

    auto applyOutput = repo.executeGitCommand("apply", "--cached", "--3way", repo.getGitDirectoryPath() / "patch.diff");

    std::filesystem::remove(repo.getGitDirectoryPath() / "patch.diff");

    if (applyOutput.return_code != 0)
    {
        auto index = repo.Index();
        auto unmergedFilesEntries = index.getUnmergedFilesListWithDetails();

        if (!unmergedFilesEntries.empty())
        {
            auto merge = Merge{ repo };
            merge.threeWayMergeConflictedFiles(unmergedFilesEntries, commitHash, "HEAD");
            createCherryPickHeadFile(commitHash);
            throw std::runtime_error("Conflicts detected");
        }

        throw std::runtime_error("Failed to apply diff");
    }

    auto checkoutIndexOutput = repo.executeGitCommand("checkout-index", "-a", "-f");

    if (checkoutIndexOutput.return_code != 0)
    {
        throw std::runtime_error("Failed to checkout index");
    }

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
    std::ofstream headFile(repo.getGitDirectoryPath() / "CHERRY_PICK_HEAD");
    headFile << commitHash;
    headFile.close();
}

auto CherryPick::createConflictMsgFiles(const std::string_view message, const std::string_view description) const -> void
{
    auto messageAndDescription = std::string{ message };
    if (!description.empty())
    {
        messageAndDescription += "\n\n" + std::string{ description };
    }

    auto mergeMsgFile = std::ofstream(repo.getGitDirectoryPath() / "MERGE_MSG");
    mergeMsgFile << messageAndDescription;
    mergeMsgFile.close();

    auto commitEditMsgFile = std::ofstream(repo.getGitDirectoryPath() / "COMMIT_EDITMSG");
    commitEditMsgFile << messageAndDescription;
    commitEditMsgFile.close();
}

auto CherryPick::getCherryPickHead() const -> std::string
{
    std::ifstream headFile(repo.getGitDirectoryPath() / "CHERRY_PICK_HEAD");

    std::string head;
    headFile >> head;
    headFile.close();

    return head;
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

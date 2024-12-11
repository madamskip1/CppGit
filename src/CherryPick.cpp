#include "CherryPick.hpp"

#include "Commits.hpp"
#include "Index.hpp"
#include "_details/FileUtility.hpp"

#include <expected>
#include <filesystem>

namespace CppGit {

CherryPick::CherryPick(const Repository& repo)
    : repo(repo),
      _createCommit(repo),
      _threeWayMerge(repo),
      _applyDiff(repo)
{
}

auto CherryPick::cherryPickCommit(const std::string_view commitHash, CherryPickEmptyCommitStrategy emptyCommitStrategy) const -> std::expected<std::string, Error>
{
    auto index = repo.Index();
    if (index.isDirty())
    {
        return std::unexpected{ Error::DIRTY_WORKTREE };
    }

    auto applyDiffResult = _applyDiff.apply(commitHash);

    if (applyDiffResult == _details::ApplyDiffResult::EMPTY_DIFF || applyDiffResult == _details::ApplyDiffResult::NO_CHANGES)
    {
        return processEmptyDiff(commitHash, emptyCommitStrategy);
    }
    if (applyDiffResult == _details::ApplyDiffResult::CONFLICT)
    {
        auto unmergedFilesEntries = index.getUnmergedFilesListWithDetails();
        _threeWayMerge.mergeConflictedFiles(unmergedFilesEntries, commitHash, "HEAD");
        createCherryPickHeadFile(commitHash);

        return std::unexpected{ Error::CHERRY_PICK_CONFLICT };
    }

    return commitCherryPicked(commitHash);
}

auto CherryPick::commitEmptyCherryPickedCommit() const -> std::expected<std::string, Error>
{
    auto commitHash = getCherryPickHead();
    if (commitHash.empty())
    {
        return std::unexpected{ Error::NO_CHERRY_PICK_IN_PROGRESS };
    }

    auto commitedHash = commitCherryPicked(commitHash);
    std::filesystem::remove(repo.getGitDirectoryPath() / "CHERRY_PICK_HEAD");

    return commitedHash;
}

auto CherryPick::isCherryPickInProgress() const -> bool
{
    if (std::ifstream headFile(repo.getGitDirectoryPath() / "CHERRY_PICK_HEAD"); headFile.is_open())
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

auto CherryPick::cherryPickContinue() const -> std::expected<std::string, Error>
{
    if (!isCherryPickInProgress())
    {
        return std::unexpected{ Error::NO_CHERRY_PICK_IN_PROGRESS };
    }

    auto unmergedFilesEntries = repo.Index().getUnmergedFilesListWithDetails();

    if (!unmergedFilesEntries.empty())
    {
        return std::unexpected{ Error::CHERRY_PICK_CONFLICT };
    }

    auto commitHash = getCherryPickHead();

    return commitCherryPicked(commitHash);
}

auto CppGit::CherryPick::processEmptyDiff(const std::string_view commitHash, CherryPickEmptyCommitStrategy emptyCommitStrategy) const -> std::expected<std::string, Error>
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
        return std::unexpected{ Error::CHERRY_PICK_EMPTY_COMMIT };
    }
}

} // namespace CppGit

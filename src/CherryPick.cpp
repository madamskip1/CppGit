#include "CherryPick.hpp"

#include "Commits.hpp"
#include "Error.hpp"
#include "Index.hpp"
#include "Repository.hpp"
#include "Reset.hpp"
#include "_details/ApplyDiff.hpp"
#include "_details/FileUtility.hpp"
#include "_details/GitFilesHelper.hpp"
#include "_details/Refs.hpp"

#include <cstddef>
#include <expected>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

namespace CppGit {

CherryPick::CherryPick(const Repository& repo)
    : repo(repo),
      _createCommit(repo),
      _applyDiff(repo)
{
}

auto CherryPick::cherryPickCommit(const std::string_view commitHash, CherryPickEmptyCommitStrategy emptyCommitStrategy) const -> std::expected<std::string, Error>
{
    if (repo.Index().isDirty())
    {
        return std::unexpected{ Error::DIRTY_WORKTREE };
    }

    _details::GitFilesHelper{ repo }.setOrigHeadFile(repo.Commits().getHeadCommitHash());

    auto applyDiffResult = _applyDiff.apply(commitHash);

    if (applyDiffResult == _details::ApplyDiffResult::EMPTY_DIFF || applyDiffResult == _details::ApplyDiffResult::NO_CHANGES)
    {
        return processEmptyDiff(commitHash, emptyCommitStrategy);
    }
    if (applyDiffResult == _details::ApplyDiffResult::CONFLICT)
    {
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
    removeCherryPickHeadFile();

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
    auto newCommitHash = _createCommit.createCommit(commitInfo.getMessage(), commitInfo.getDescription(), { parent }, envp);
    _details::Refs{ repo }.updateRefHash("HEAD", newCommitHash);

    return newCommitHash;
}

auto CherryPick::createCherryPickHeadFile(const std::string_view commitHash) const -> void
{
    _details::FileUtility::createOrOverwriteFile(repo.getGitDirectoryPath() / "CHERRY_PICK_HEAD", commitHash);
}

auto CherryPick::getCherryPickHead() const -> std::string
{
    return _details::FileUtility::readFile(repo.getGitDirectoryPath() / "CHERRY_PICK_HEAD");
}

auto CherryPick::removeCherryPickHeadFile() const -> void
{
    std::filesystem::remove(repo.getGitDirectoryPath() / "CHERRY_PICK_HEAD");
}

auto CherryPick::cherryPickContinue() const -> std::expected<std::string, Error>
{
    if (!isCherryPickInProgress())
    {
        return std::unexpected{ Error::NO_CHERRY_PICK_IN_PROGRESS };
    }


    if (auto unmergedFilesEntries = repo.Index().getUnmergedFilesListWithDetails(); !unmergedFilesEntries.empty())
    {
        return std::unexpected{ Error::CHERRY_PICK_CONFLICT };
    }

    auto commitHash = getCherryPickHead();
    removeCherryPickHeadFile();
    return commitCherryPicked(commitHash);
}

auto CherryPick::cherryPickAbort() const -> Error
{
    if (!isCherryPickInProgress())
    {
        return Error::NO_CHERRY_PICK_IN_PROGRESS;
    }

    Reset{ repo }.resetHard("HEAD");
    removeCherryPickHeadFile();

    return Error::NO_ERROR;
}

auto CppGit::CherryPick::processEmptyDiff(const std::string_view commitHash, CherryPickEmptyCommitStrategy emptyCommitStrategy) const -> std::expected<std::string, Error>
{
    switch (emptyCommitStrategy)
    {
    case CherryPickEmptyCommitStrategy::DROP: {
        constexpr auto HASH_LENGTH = std::size_t{ 40 };
        return std::string(HASH_LENGTH, '0');
    }
    case CherryPickEmptyCommitStrategy::KEEP:
        return commitCherryPicked(commitHash);
    case CherryPickEmptyCommitStrategy::STOP:
        createCherryPickHeadFile(commitHash);
        return std::unexpected{ Error::CHERRY_PICK_EMPTY_COMMIT };
    [[unlikely]] default:
        throw std::logic_error{ "Unknown CherryPickEmptyCommitStrategy" };
    }
}

} // namespace CppGit

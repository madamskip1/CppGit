#include "CppGit/CherryPicker.hpp"

#include "CppGit/CommitsManager.hpp"
#include "CppGit/IndexManager.hpp"
#include "CppGit/Repository.hpp"
#include "CppGit/Resetter.hpp"
#include "CppGit/_details/CommitCreator.hpp"
#include "CppGit/_details/DiffApplier.hpp"
#include "CppGit/_details/FileUtility.hpp"
#include "CppGit/_details/GitFilesHelper.hpp"
#include "CppGit/_details/ReferencesManager.hpp"

#include <cstddef>
#include <expected>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

namespace CppGit {

CherryPicker::CherryPicker(const Repository& repository)
    : repository{ &repository }
{
}

auto CherryPicker::cherryPick(const std::string_view commitHash, const CherryPickEmptyCommitStrategy emptyCommitStrategy) const -> std::expected<std::string, CherryPickResult>
{
    _details::GitFilesHelper{ *repository }.setOrigHeadFile(repository->CommitsManager().getHeadCommitHash());

    const auto applyDiffResult = _details::DiffApplier{ *repository }.applyDiff(commitHash);

    if (applyDiffResult == _details::ApplyDiffResult::EMPTY_DIFF || applyDiffResult == _details::ApplyDiffResult::NO_CHANGES)
    {
        return processEmptyDiff(commitHash, emptyCommitStrategy);
    }
    if (applyDiffResult == _details::ApplyDiffResult::CONFLICT)
    {
        createCherryPickHeadFile(commitHash);
        return std::unexpected{ CherryPickResult::CONFLICT };
    }

    return commitCherryPicked(commitHash);
}

auto CherryPicker::commitEmptyCherryPickedCommit() const -> std::expected<std::string, CherryPickResult>
{
    const auto commitHash = getCherryPickHead();
    auto commitedHash = commitCherryPicked(commitHash);
    removeCherryPickHeadFile();

    return commitedHash;
}

auto CherryPicker::isCherryPickInProgress() const -> bool
{
    if (std::ifstream headFile(repository->getGitDirectoryPath() / "CHERRY_PICK_HEAD"); headFile.is_open())
    {
        const auto isEmpty = headFile.peek() == std::ifstream::traits_type::eof();
        headFile.close();

        return !isEmpty;
    }

    return false;
}

auto CherryPicker::commitCherryPicked(const std::string_view commitHash) const -> std::string
{
    const auto commitsManager = repository->CommitsManager();

    const auto commitInfo = commitsManager.getCommitInfo(commitHash);

    auto envp = std::vector<std::string>{
        "GIT_AUTHOR_NAME=" + commitInfo.getAuthor().name,
        "GIT_AUTHOR_EMAIL=" + commitInfo.getAuthor().email,
        "GIT_AUTHOR_DATE=" + commitInfo.getAuthorDate()
    };

    auto parent = commitsManager.hasAnyCommits() ? commitsManager.getHeadCommitHash() : std::string{};
    auto newCommitHash = _details::CommitCreator{ *repository }.createCommit(commitInfo.getMessage(), commitInfo.getDescription(), { parent }, envp);
    _details::ReferencesManager{ *repository }.updateRefHash("HEAD", newCommitHash);

    return newCommitHash;
}

auto CherryPicker::createCherryPickHeadFile(const std::string_view commitHash) const -> void
{
    _details::FileUtility::createOrOverwriteFile(repository->getGitDirectoryPath() / "CHERRY_PICK_HEAD", commitHash);
}

auto CherryPicker::getCherryPickHead() const -> std::string
{
    return _details::FileUtility::readFile(repository->getGitDirectoryPath() / "CHERRY_PICK_HEAD");
}

auto CherryPicker::removeCherryPickHeadFile() const -> void
{
    std::filesystem::remove(repository->getGitDirectoryPath() / "CHERRY_PICK_HEAD");
}

auto CherryPicker::continueCherryPick() const -> std::expected<std::string, CherryPickResult>
{
    if (const auto unmergedFilesEntries = repository->IndexManager().getUnmergedFilesDetailedList(); !unmergedFilesEntries.empty())
    {
        return std::unexpected{ CherryPickResult::CONFLICT };
    }

    const auto commitHash = getCherryPickHead();
    removeCherryPickHeadFile();
    return commitCherryPicked(commitHash);
}

auto CherryPicker::abortCherryPick() const -> void
{
    repository->Resetter().resetHard("HEAD");
    removeCherryPickHeadFile();
}

auto CppGit::CherryPicker::processEmptyDiff(const std::string_view commitHash, const CherryPickEmptyCommitStrategy emptyCommitStrategy) const -> std::expected<std::string, CherryPickResult>
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
    default:
        createCherryPickHeadFile(commitHash);
        return std::unexpected{ CherryPickResult::EMPTY_COMMIT_OR_EMPTY_DIFF };
    }
}

} // namespace CppGit

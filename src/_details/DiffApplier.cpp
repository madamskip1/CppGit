#include "CppGit/_details/DiffApplier.hpp"

#include "CppGit/IndexManager.hpp"
#include "CppGit/Repository.hpp"
#include "CppGit/_details/FileUtility.hpp"
#include "CppGit/_details/IndexWorktreeManager.hpp"
#include "CppGit/_details/Parser/DiffParser.hpp"
#include "CppGit/_details/ThreeWayMerger.hpp"

#include <filesystem>
#include <string_view>
#include <utility>

namespace CppGit::_details {

DiffApplier::DiffApplier(const Repository& repository)
    : repository{ &repository }
{
}

auto DiffApplier::applyDiff(const std::string_view commitHash) const -> ApplyDiffResult
{
    auto diff = getDiff(commitHash);

    if (diff.empty())
    {
        return ApplyDiffResult::EMPTY_DIFF;
    }

    createMissingFilesThatOccurInPatch(diff);

    const auto patchDiffPath = repository->getGitDirectoryPath() / "patch.diff";

    _details::FileUtility::createOrOverwriteFile(patchDiffPath, std::move(diff));
    auto applyOutput = repository->executeGitCommand("apply", "--cached", "--3way", patchDiffPath);
    std::filesystem::remove(patchDiffPath);
    IndexWorktreeManager{ *repository }.copyForceIndexToWorktree();

    if (applyOutput.return_code == 0)
    {
        if (repository->IndexManager().getStagedFilesList().empty())
        {
            return ApplyDiffResult::NO_CHANGES;
        }
    }
    else
    {
        if (applyOutput.return_code == 1 && applyOutput.stderr.contains("conflicts"))
        {
            const auto unmergedFilesEntries = repository->IndexManager().getUnmergedFilesDetailedList();
            if (!unmergedFilesEntries.empty())
            {
                ThreeWayMerger{ *repository }.mergeConflictedFiles(unmergedFilesEntries, commitHash, "HEAD");
                return ApplyDiffResult::CONFLICT;
            }
        }
    }

    return ApplyDiffResult::APPLIED;
}

auto DiffApplier::getDiff(const std::string_view commitHash) const -> std::string
{
    auto output = repository->executeGitCommand("diff-tree", "--patch", commitHash);
    return std::move(output.stdout);
}


auto DiffApplier::createMissingFilesThatOccurInPatch(const std::string_view diff) const -> void
{
    auto diffParser = DiffParser{};
    const auto diffFiles = diffParser.parse(diff);
    const auto indexManager = repository->IndexManager();
    for (const auto& diffFile : diffFiles)
    {
        auto filePath = repository->getPath() / diffFile.fileA;
        if (!std::filesystem::exists(filePath))
        {
            _details::FileUtility::createOrOverwriteFile(filePath, "");
            indexManager.add(diffFile.fileA);
        }
    }
}
} // namespace CppGit::_details

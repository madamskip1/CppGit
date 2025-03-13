#include "CppGit/_details/ApplyDiff.hpp"

#include "CppGit/Index.hpp"
#include "CppGit/Repository.hpp"
#include "CppGit/_details/FileUtility.hpp"
#include "CppGit/_details/IndexWorktree.hpp"
#include "CppGit/_details/Parser/DiffParser.hpp"
#include "CppGit/_details/ThreeWayMerge.hpp"

#include <filesystem>
#include <string_view>
#include <utility>

namespace CppGit::_details {

ApplyDiff::ApplyDiff(const Repository& repo)
    : repo{ &repo }
{
}

auto ApplyDiff::apply(const std::string_view commitHash) const -> ApplyDiffResult
{
    auto diff = getDiff(commitHash);

    if (diff.empty())
    {
        return ApplyDiffResult::EMPTY_DIFF;
    }

    createMissingFilesThatOccurInPatch(diff);

    const auto patchDiffPath = repo->getGitDirectoryPath() / "patch.diff";

    _details::FileUtility::createOrOverwriteFile(patchDiffPath, std::move(diff));
    auto applyOutput = repo->executeGitCommand("apply", "--cached", "--3way", patchDiffPath);
    std::filesystem::remove(patchDiffPath);
    IndexWorktree{ *repo }.copyForceIndexToWorktree();

    if (applyOutput.return_code == 0)
    {
        if (repo->Index().getStagedFilesList().empty())
        {
            return ApplyDiffResult::NO_CHANGES;
        }
    }
    else
    {
        if (applyOutput.return_code == 1 && applyOutput.stderr.contains("conflicts"))
        {
            const auto unmergedFilesEntries = repo->Index().getUnmergedFilesListWithDetails();
            if (!unmergedFilesEntries.empty())
            {
                const auto threeWayMerge = ThreeWayMerge{ *repo };
                threeWayMerge.mergeConflictedFiles(unmergedFilesEntries, commitHash, "HEAD");
                return ApplyDiffResult::CONFLICT;
            }
        }
    }

    return ApplyDiffResult::APPLIED;
}

auto ApplyDiff::getDiff(const std::string_view commitHash) const -> std::string
{
    auto output = repo->executeGitCommand("diff-tree", "--patch", commitHash);
    return std::move(output.stdout);
}


auto ApplyDiff::createMissingFilesThatOccurInPatch(const std::string_view diff) const -> void
{
    auto diffParser = DiffParser{};
    const auto diffFiles = diffParser.parse(diff);
    const auto index = repo->Index();
    for (const auto& diffFile : diffFiles)
    {
        auto filePath = repo->getPath() / diffFile.fileA;
        if (!std::filesystem::exists(filePath))
        {
            _details::FileUtility::createOrOverwriteFile(filePath, "");
            index.add(diffFile.fileA);
        }
    }
}
} // namespace CppGit::_details

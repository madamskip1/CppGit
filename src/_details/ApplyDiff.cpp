#include "_details/ApplyDiff.hpp"

#include "Index.hpp"
#include "Repository.hpp"
#include "_details/FileUtility.hpp"
#include "_details/IndexWorktree.hpp"
#include "_details/Parser/DiffParser.hpp"
#include "_details/ThreeWayMerge.hpp"

#include <filesystem>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace CppGit::_details {

ApplyDiff::ApplyDiff(const Repository& repo)
    : repo(repo),
      patchDiffPath(repo.getGitDirectoryPath() / "patch.diff")
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

    _details::FileUtility::createOrOverwriteFile(patchDiffPath, diff);
    auto applyOutput = repo.executeGitCommand("apply", "--cached", "--3way", patchDiffPath);
    std::filesystem::remove(patchDiffPath);
    IndexWorktree{ repo }.copyForceIndexToWorktree();

    if (applyOutput.return_code == 0)
    {
        if (repo.Index().getStagedFilesList().empty())
        {
            return ApplyDiffResult::NO_CHANGES;
        }
    }
    else
    {
        if (applyOutput.return_code == 1 && applyOutput.stderr.contains("conflicts"))
        {
            auto unmergedFilesEntries = repo.Index().getUnmergedFilesListWithDetails();
            if (!unmergedFilesEntries.empty())
            {
                auto threeWayMerge = ThreeWayMerge{ repo };
                threeWayMerge.mergeConflictedFiles(unmergedFilesEntries, commitHash, "HEAD");
                return ApplyDiffResult::CONFLICT;
            }
        }

        throw std::runtime_error("Failed to apply diff");
    }

    return ApplyDiffResult::APPLIED;
}

auto ApplyDiff::getDiff(const std::string_view commitHash) const -> std::string
{
    auto output = repo.executeGitCommand("diff-tree", "--patch", commitHash);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to get diff");
    }

    return std::move(output.stdout);
}


auto ApplyDiff::createMissingFilesThatOccurInPatch(const std::string_view diff) const -> void
{
    auto diffParser = DiffParser{};
    auto diffFiles = diffParser.parse(diff);
    auto index = repo.Index();
    for (const auto& diffFile : diffFiles)
    {
        auto filePath = repo.getPath() / diffFile.fileA;
        if (!std::filesystem::exists(filePath))
        {
            _details::FileUtility::createOrOverwriteFile(filePath, "");
            index.add(diffFile.fileA);
        }
    }
}
} // namespace CppGit::_details

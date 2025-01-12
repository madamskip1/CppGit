#include "_details/ApplyDiff.hpp"

#include "Index.hpp"
#include "_details/FileUtility.hpp"
#include "_details/IndexWorktree.hpp"
#include "_details/Parser/DiffParser.hpp"

#include <filesystem>

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
        if (applyOutput.return_code == 1 && applyOutput.stderr.find("conflicts") != std::string::npos)
        {
            return ApplyDiffResult::CONFLICT;
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

} // namespace CppGit::_details

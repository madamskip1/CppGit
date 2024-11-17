#include "_details/ThreeWayMerge.hpp"

#include <fstream>

namespace CppGit::_details {

ThreeWayMerge::ThreeWayMerge(const Repository& repo)
    : repo(repo)
{
}

auto ThreeWayMerge::mergeConflictedFiles(const std::vector<IndexEntry>& unmergedFilesEntries, const std::string_view sourceLabel, const std::string_view targetLabel) const -> void
{
    const auto unmergedFiles = createUnmergedFileMap(unmergedFilesEntries);

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

auto ThreeWayMerge::createMergeMsgFile(const std::string_view msg, const std::string_view description) const -> void
{
    auto path = repo.getGitDirectoryPath() / "MERGE_MSG";
    auto file = std::ofstream{ path };
    file << msg;
    if (!description.empty())
    {
        file << "\n\n"
             << description;
    }
    file.close();
}

auto ThreeWayMerge::removeMergeMsgFile() const -> void
{
    auto path = repo.getGitDirectoryPath() / "MERGE_MSG";
    std::filesystem::remove(path);
}

auto ThreeWayMerge::unpackFile(const std::string_view fileBlob) const -> std::string
{
    if (fileBlob.empty())
    {
        return std::string{};
    }

    auto output = repo.executeGitCommand("unpack-file", fileBlob);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to unpack file");
    }

    return std::move(output.stdout);
}

auto ThreeWayMerge::createUnmergedFileMap(const std::vector<IndexEntry>& unmergedFilesEntries) const -> std::unordered_map<std::string, UnmergedFileBlobs>
{
    std::unordered_map<std::string, UnmergedFileBlobs> unmergedFiles;

    for (const auto& indexEntry : unmergedFilesEntries)
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

} // namespace CppGit::_details

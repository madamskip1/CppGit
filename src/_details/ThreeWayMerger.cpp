#include "CppGit/_details/ThreeWayMerger.hpp"

#include "CppGit/IndexManager.hpp"
#include "CppGit/Repository.hpp"

#include <filesystem>
#include <fstream>
#include <ios>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace CppGit::_details {

ThreeWayMerger::ThreeWayMerger(const Repository& repository)
    : repository{ &repository }
{
}

auto ThreeWayMerger::mergeConflictedFiles(const std::vector<IndexEntry>& unmergedFilesEntries, const std::string_view sourceLabel, const std::string_view targetLabel) const -> void
{
    auto unmergedFiles = createUnmergedFileMap(unmergedFilesEntries);

    const auto repoRootPath = repository->getTopLevelPath();

    for (const auto& [file, unmergedFile] : unmergedFiles)
    {
        auto baseTempFile = unpackFile(unmergedFile.baseBlob);
        auto targetTempFile = unpackFile(unmergedFile.targetBlob);
        auto sourceTempFile = unpackFile(unmergedFile.sourceBlob);

        if (baseTempFile.empty())
        {
            baseTempFile = "/dev/null";
        }

        repository->executeGitCommand("merge-file", "-L", targetLabel, "-L", "ancestor", "-L", sourceLabel, targetTempFile, baseTempFile, sourceTempFile);

        repository->executeGitCommand("checkout-index", "-f", "--stage=2", "--", file);

        auto baseTempFilePath = std::filesystem::path{ repoRootPath / baseTempFile };

        auto targetTempFilePath = std::filesystem::path{ repoRootPath / std::move(targetTempFile) };
        auto sourceTempFilePath = std::filesystem::path{ repoRootPath / std::move(sourceTempFile) };
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

auto ThreeWayMerger::createMergeMsgFile(const std::string_view msg, const std::string_view description) const -> void
{
    const auto path = repository->getGitDirectoryPath() / "MERGE_MSG";
    auto file = std::ofstream{ path };
    file << msg;
    if (!description.empty())
    {
        file << "\n\n"
             << description;
    }
    file.close();
}

auto ThreeWayMerger::removeMergeMsgFile() const -> void
{
    const auto path = repository->getGitDirectoryPath() / "MERGE_MSG";
    std::filesystem::remove(path);
}


auto ThreeWayMerger::getMergeMsg() const -> std::string
{
    const auto path = repository->getGitDirectoryPath() / "MERGE_MSG";
    auto file = std::ifstream{ path };
    auto msg = std::string{};
    std::getline(file, msg, '\0');
    file.close();

    return msg;
}

auto ThreeWayMerger::unpackFile(const std::string_view fileBlob) const -> std::string
{
    if (fileBlob.empty())
    {
        return std::string{};
    }

    auto output = repository->executeGitCommand("unpack-file", fileBlob);
    return std::move(output.stdout);
}

auto ThreeWayMerger::createUnmergedFileMap(const std::vector<IndexEntry>& unmergedFilesEntries) -> std::unordered_map<std::string, UnmergedFileBlobs>
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

#include "Index.hpp"

#include "Commit.hpp"
#include "Commits.hpp"
#include "Parser/IndexParser.hpp"
#include "Repository.hpp"

#include <algorithm>

namespace CppGit {

Index::Index(const Repository& repo)
    : repo(repo)
{
}

auto Index::add(const std::filesystem::path& path) const -> void
{
    const auto& absolutePath = path.is_absolute() ? path : repo.getAbsoluteFromRelativePath(path);

    if (!std::filesystem::exists(absolutePath))
    {
        throw std::runtime_error("File does not exist");
    }

    if (repo.isPathInGitDirectory(absolutePath))
    {
        throw std::runtime_error("Cannot add file from git directory");
    }

    if (std::filesystem::is_directory(absolutePath))
    {
        for (const auto& entryAboslutePath : std::filesystem::recursive_directory_iterator(absolutePath))
        {
            if (!std::filesystem::is_directory(entryAboslutePath) && !repo.isPathInGitDirectory(entryAboslutePath))
            {
                const auto relativePathEntry = repo.getRelativeFromAbsolutePath(entryAboslutePath);

                addFileToIndex(relativePathEntry, entryAboslutePath);
            }
        }
    }
    else
    {
        const auto& relativePath = path.is_relative() ? path : repo.getRelativeFromAbsolutePath(path);

        addFileToIndex(relativePath, absolutePath);
    }
}

auto Index::remove(const std::filesystem::path& path) const -> void
{
    const auto& aboslutePath = path.is_absolute() ? path : repo.getAbsoluteFromRelativePath(path);

    if (!std::filesystem::exists(aboslutePath))
    {
        throw std::runtime_error("File does not exist");
    }

    if (repo.isPathInGitDirectory(aboslutePath))
    {
        throw std::runtime_error("Cannot remove file from git directory");
    }

    if (std::filesystem::is_directory(aboslutePath))
    {
        for (const auto& entryAboslutePath : std::filesystem::recursive_directory_iterator(aboslutePath))
        {
            if (!std::filesystem::is_directory(entryAboslutePath) && !repo.isPathInGitDirectory(entryAboslutePath))
            {
                const auto relativePathEntry = repo.getRelativeFromAbsolutePath(entryAboslutePath);

                removeFileFromIndex(relativePathEntry);
            }
        }
    }
    else
    {
        const auto& relativePath = path.is_relative() ? path : repo.getRelativeFromAbsolutePath(path);

        removeFileFromIndex(relativePath);
    }
}

auto Index::reset() const -> void
{
    const auto filesInIndex = getFilesInIndexList();
    for (const auto& file : filesInIndex)
    {
        removeFileFromIndex(file);
    }
}

auto Index::isFileStaged(const std::filesystem::path& path) const -> bool
{
    auto output = GitCommandExecutorUnix().execute(repo.getPathAsString(), "diff-index", "--cached", "--name-only", "HEAD", "--", path.string());

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to list staged files");
    }

    return output.stdout == path.string();
}

auto Index::getFilesInIndexList() const -> std::vector<std::string>
{
    auto output = GitCommandExecutorUnix().execute(repo.getPathAsString(), "ls-files", "--cache");

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to list staged files");
    }

    return IndexParser::parseStageSimpleCacheList(output.stdout);
}

auto Index::getFilesInIndexListWithDetails() const -> std::vector<IndexEntry>
{
    auto output = GitCommandExecutorUnix().execute(repo.getPathAsString(), "ls-files", "--stage");

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to list staged files");
    }

    return IndexParser::parseStageDetailedList(output.stdout);
}

auto Index::getUntrackedFilesList() const -> std::vector<std::string>
{
    auto output = GitCommandExecutorUnix().execute(repo.getPathAsString(), "ls-files", "--others", "--exclude-standard");

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to list untracked files");
    }

    if (output.stdout.empty())
    {
        return {};
    }

    auto splittedList_SV = Parser::split(output.stdout, '\n');
    std::vector<std::string> untrackedFilesList;
    untrackedFilesList.reserve(splittedList_SV.size());

    for (const auto file : splittedList_SV)
    {
        untrackedFilesList.emplace_back(file);
    }

    return untrackedFilesList;
}

auto Index::getStagedFilesList() const -> std::vector<DiffIndexEntry>
{
    auto output = GitCommandExecutorUnix().execute(repo.getPathAsString(), "diff-index", "--cached", "--name-status", "HEAD", "--");

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to list staged files");
    }

    return IndexParser::parseDiffIndexWithStatus(output.stdout);
}

auto Index::getNotStagedFilesList() const -> std::vector<std::string>
{
    auto output = GitCommandExecutorUnix().execute(repo.getPathAsString(), "ls-files", "--modified", "--deleted", "--others", "--exclude-standard", "--deduplicate");

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to list not staged files");
    }

    if (output.stdout.empty())
    {
        return std::vector<std::string>{};
    }

    auto splittedSV = Parser::split(output.stdout, '\n');

    return std::vector<std::string>{ splittedSV.cbegin(), splittedSV.cend() };
}

auto Index::isDirty() const -> bool
{
    auto commits = repo.Commits();
    auto headCommit = commits.getCommitInfo("HEAD");

    auto output = repo.executeGitCommand("diff-index", "--quiet", "--exit-code", headCommit.getTreeHash());
    if (output.return_code > 1)
    {
        throw std::runtime_error("Failed to check if index is dirty");
    }

    return output.return_code == 1;
}

auto Index::getFileMode(const std::filesystem::path& absolutePath) -> std::string
{
    if (std::filesystem::is_symlink(absolutePath))
    {
        return "120000";
    }

    if (!std::filesystem::is_regular_file(absolutePath))
    {
        throw std::runtime_error("Unsupported file type");
    }

    if (auto permissions = std::filesystem::status(absolutePath).permissions();
        (permissions & std::filesystem::perms::owner_exec) != std::filesystem::perms::none)
    {
        return "100755";
    }

    return "100644";
}

auto Index::addFileToIndex(const std::filesystem::path& relativePath, const std::filesystem::path& absolutePath) const -> void
{
    auto hashOutput = GitCommandExecutorUnix().execute(repo.getPathAsString(), "hash-object", "-w", relativePath.string());

    if (hashOutput.return_code != 0)
    {
        throw std::runtime_error("Failed to hash object");
    }

    const auto& objectHash = hashOutput.stdout;
    const auto fileMode = getFileMode(absolutePath);

    auto updateIndexOutput = GitCommandExecutorUnix().execute(repo.getPathAsString(), "update-index", "--add", "--cacheinfo", fileMode, objectHash, relativePath.string());

    if (updateIndexOutput.return_code != 0)
    {
        throw std::runtime_error("Failed to update index");
    }
}

auto Index::removeFileFromIndex(const std::filesystem::path& relativePath) const -> void
{
    auto output = GitCommandExecutorUnix().execute(repo.getPathAsString(), "update-index", "--force-remove", relativePath.string());

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to remove file from index");
    }
}

} // namespace CppGit

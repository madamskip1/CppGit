#include "Index.hpp"

#include "Parser/IndexParser.hpp"
#include "Repository.hpp"

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
    const auto filesInIndex = getStagedFilesList();
    for (const auto& file : filesInIndex)
    {
        removeFileFromIndex(file);
    }
}

auto Index::isFileStaged(const std::filesystem::path& path) const -> bool
{
    const auto& relativePath = path.is_relative() ? path : repo.getRelativeFromAbsolutePath(path);
    const auto output = GitCommandExecutorUnix().execute(repo.getPathAsString(), "ls-files", "--cache", relativePath.string());

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to check if file is staged");
    }

    return output.stdout == relativePath.string();
}

auto Index::getStagedFilesList() const -> std::vector<std::string>
{
    auto output = GitCommandExecutorUnix().execute(repo.getPathAsString(), "ls-files", "--cache");

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to list staged files");
    }

    return IndexParser::parseStageSimpleCacheList(output.stdout);
}

auto Index::getStagedFilesListWithDetails() const -> std::vector<IndexEntry>
{
    auto output = GitCommandExecutorUnix().execute(repo.getPathAsString(), "ls-files", "--stage");

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to list staged files");
    }

    return IndexParser::parseStageDetailedList(output.stdout);
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

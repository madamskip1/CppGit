#include "Index.hpp"

#include "Parser/IndexParser.hpp"

namespace CppGit {

Index::Index(const Repository& repo)
    : repo(repo)
{
}

void Index::add(const std::filesystem::path& path) const
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

std::vector<std::string> Index::getStagedFilesList() const
{
    auto output = GitCommandExecutorUnix().execute(repo.getPathAsString(), "ls-files", "--cache");

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to list staged files");
    }

    return IndexParser::parseStageSimpleCacheList(output.stdout);
}

std::vector<IndexEntry> Index::getStagedFilesListWithDetails() const
{
    auto output = GitCommandExecutorUnix().execute(repo.getPathAsString(), "ls-files", "--stage");

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to list staged files");
    }

    return IndexParser::parseStageDetailedList(output.stdout);
}

std::string Index::getFileMode(const std::filesystem::path& absolutePath)
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

void Index::addFileToIndex(const std::filesystem::path& relativePath, const std::filesystem::path& absolutePath) const
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

} // namespace CppGit

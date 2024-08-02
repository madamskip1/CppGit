#include "Index.hpp"

#include "Parser/IndexParser.hpp"

namespace CppGit {

Index::Index(const Repository& repo)
    : repo(repo)
{
}

void Index::add(const std::filesystem::path& path)
{
    if (!std::filesystem::exists(path))
    {
        throw std::runtime_error("File does not exist");
    }

    if (isPathInGitDirectory(path))
    {
        throw std::runtime_error("Cannot add file from git directory");
    }

    if (std::filesystem::is_directory(path))
    {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
        {
            if (!std::filesystem::is_directory(entry) && !isPathInGitDirectory(entry))
            {
                addFileToIndex(entry);
            }
        }
    }
    else
    {
        addFileToIndex(path);
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

std::string Index::getFileMode(const std::filesystem::path& path)
{
    if (std::filesystem::is_symlink(path))
    {
        return "120000";
    }

    if (!std::filesystem::is_regular_file(path))
    {
        throw std::runtime_error("Unsupported file type");
    }

    if (auto permissions = std::filesystem::status(path).permissions();
        (permissions & std::filesystem::perms::owner_exec) != std::filesystem::perms::none)
    {
        return "100755";
    }

    return "100644";
}

void Index::addFileToIndex(const std::filesystem::path& path)
{
    auto hashOutput = GitCommandExecutorUnix().execute(repo.getPathAsString(), "hash-object", "-w", path.string());
    if (hashOutput.return_code != 0)
    {
        throw std::runtime_error("Failed to hash object");
    }

    const auto& objectHash = hashOutput.stdout;
    auto fileMode = getFileMode(path);

    auto dirPath = path.parent_path();
    auto topLevelPath = repo.getTopLevelPath();
    auto relativePathToDir = std::filesystem::relative(dirPath, repo.getTopLevelPath());
    std::filesystem::path relativePath = "";

    if (relativePathToDir != ".")
    {
        relativePath = relativePathToDir / path.filename();
    }
    else
    {
        relativePath = path.filename();
    }

    auto updateIndexOutput = GitCommandExecutorUnix().execute(repo.getPathAsString(), "update-index", "--add", "--cacheinfo", fileMode, objectHash, relativePath.string());

    if (updateIndexOutput.return_code != 0)
    {
        throw std::runtime_error("Failed to update index");
    }
}

bool Index::isPathInGitDirectory(const std::filesystem::path& path) const
{
    const auto canonicalGitDir = std::filesystem::canonical(repo.getGitDirectoryPath());
    const auto canonicalArgPath = std::filesystem::canonical(path);

    const auto canonicalGitDirStr = canonicalGitDir.string();
    const auto canonicalArgPathStr = canonicalArgPath.string();

    if (canonicalArgPathStr.size() < canonicalGitDirStr.size())
    {
        return false;
    }

    return std::mismatch(canonicalGitDirStr.begin(), canonicalGitDirStr.end(), canonicalArgPathStr.begin()).first == canonicalGitDirStr.end();
}

} // namespace CppGit

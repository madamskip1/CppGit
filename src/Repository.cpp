#include "Repository.hpp"

#include "GitCommandExecutor/GitCommandExecutorUnix.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

namespace CppGit {

Repository::Repository(const std::filesystem::path& path)
    : path(path)
{ }

auto Repository::Branches() const -> CppGit::Branches
{
    return CppGit::Branches(*this);
}

auto Repository::Index() const -> CppGit::Index
{
    return CppGit::Index(*this);
}

auto Repository::Commits() const -> CppGit::Commits
{
    return CppGit::Commits(*this);
}

auto Repository::CommitsHistory() const -> CppGit::CommitsHistory
{
    return CppGit::CommitsHistory(*this);
}

auto Repository::Diff() const -> CppGit::Diff
{
    return CppGit::Diff(*this);
}

auto Repository::getPathAsString() const -> std::string
{
    return path.string();
}

auto Repository::getPath() const -> std::filesystem::path
{
    return path;
}

auto Repository::getTopLevelPathAsString() const -> std::string
{
    return getTopLevelPath().string();
}

auto Repository::getTopLevelPath() const -> std::filesystem::path
{
    auto commandExecutor = GitCommandExecutorUnix();

    auto output = commandExecutor.execute(path.string(), "rev-parse", "--show-toplevel");

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to get top level path");
    }

    if (output.stdout.find('\n') != std::string::npos)
    {
        output.stdout.replace(output.stdout.find('\n'), 1, "");
    }

    return std::filesystem::path(output.stdout);
}

auto Repository::getGitDirectoryPath() const -> std::filesystem::path
{
    auto gitDir = getTopLevelPath() / ".git";
    return gitDir;
}

auto Repository::getAbsoluteFromRelativePath(const std::filesystem::path& relativePath) const -> std::filesystem::path
{
    return getTopLevelPath() / relativePath;
}

auto Repository::getRelativeFromAbsolutePath(const std::filesystem::path& absolutePath) const -> std::filesystem::path
{
    if (std::filesystem::is_symlink(absolutePath))
    {
        const auto symlinkDir = absolutePath.parent_path();
        const auto relativePathToSymlinkDir = std::filesystem::relative(symlinkDir, getTopLevelPath());
        if (relativePathToSymlinkDir == ".")
        {
            return absolutePath.filename();
        }

        return relativePathToSymlinkDir / absolutePath.filename();
    }

    return std::filesystem::relative(absolutePath, getTopLevelPath());
}

auto Repository::isPathInGitDirectory(const std::filesystem::path& path) const -> bool
{
    const auto canonicalGitDir = std::filesystem::canonical(getGitDirectoryPath());
    const auto canonicalArgPath = std::filesystem::canonical((path.is_absolute() ? path : getAbsoluteFromRelativePath(path)));

    const auto canonicalGitDirStr = canonicalGitDir.string();
    const auto canonicalArgPathStr = canonicalArgPath.string();

    if (canonicalArgPathStr.size() < canonicalGitDirStr.size())
    {
        return false;
    }

    return std::mismatch(canonicalGitDirStr.begin(), canonicalGitDirStr.end(), canonicalArgPathStr.begin()).first == canonicalGitDirStr.end();
}

auto Repository::isValidGitRepository() const -> bool
{
    if (!std::filesystem::exists(path))
    {
        return false;
    }

    if (auto commandExecutor = GitCommandExecutorUnix(); commandExecutor.execute(path.string(), "rev-parse", "--is-inside-work-tree").return_code != 0)
    {
        return false;
    }

    return true;
}

auto Repository::clone(const std::string& url, const std::filesystem::path& path) -> Repository
{
    auto repository = Repository(path);
    if (repository.clone(url) != ErrorCode::NO_ERROR)
    {
        throw std::runtime_error("Failed to clone repository");
    }
    return repository;
}

auto Repository::clone(const std::string& url) const -> ErrorCode
{
    if (path.empty())
    {
        return ErrorCode::GIT_CLONE_NO_PATH_GIVEN;
    }

    if (!std::filesystem::exists(path))
    {
        try
        {
            std::filesystem::create_directories(path);
        }
        catch (const std::filesystem::filesystem_error&)
        {
            return ErrorCode::GIT_CLONE_FAILED_TO_CREATE_DIRECTORIES;
        }
    }
    else
    {
        if (!std::filesystem::is_directory(path))
        {
            return ErrorCode::GIT_CLONE_PATH_IS_NOT_A_DIRECTORY;
        }
        if (!std::filesystem::is_empty(path))
        {
            return ErrorCode::GIT_CLONE_PATH_DIR_IS_NOT_EMPTY;
        }
    }

    auto commandExecutor = GitCommandExecutorUnix();

    if (auto clone_output = commandExecutor.execute(path.string(), "clone", url, path.string()); clone_output.return_code != 0)
    {
        return ErrorCode::GIT_CLONE_FAILED;
    }

    return ErrorCode::NO_ERROR;
}

auto Repository::initRepository(bool bare, std::string_view mainBranchName) const -> bool
{
    std::filesystem::path gitDir = path;
    if (!bare)
    {
        gitDir /= ".git";
    }

    if (std::filesystem::exists(path))
    {
        if (!std::filesystem::is_directory(path))
        {
            throw std::runtime_error("Path is not a directory");
            return false;
        }

        if (bare)
        {
            if (!std::filesystem::is_empty(path))
            {
                throw std::runtime_error("Directory is not empty");
                return false;
            }
        }
        else
        {
            if (std::filesystem::exists(gitDir))
            {
                throw std::runtime_error("Git directory already exists");
                return false;
            }
        }
    }

    if (!std::filesystem::create_directories(gitDir))
    {
        throw std::runtime_error("Failed to create git directory");
        return false;
    }

    if (auto objectsDir = gitDir / "objects"; !std::filesystem::create_directories(objectsDir))
    {
        throw std::runtime_error("Failed to create objects directory");
        return false;
    }

    auto refsDir = gitDir / "refs";
    if (!std::filesystem::create_directories(refsDir))
    {
        throw std::runtime_error("Failed to create refs directory");
        return false;
    }

    if (!std::filesystem::create_directory(refsDir / "heads")
        || !std::filesystem::create_directory(refsDir / "tags"))
    {
        throw std::runtime_error("Failed to create heads or tags directory");
        return false;
    }

    std::ofstream headFile(gitDir / "HEAD");
    if (!headFile.is_open())
    {
        throw std::runtime_error("Failed to create HEAD file");
        return false;
    }
    headFile << "ref: refs/heads/" << mainBranchName;
    headFile.close();

    std::string configContent = std::string{ "[core]\n" }
                              + "\trepositoryformatversion = 0\n"
                              + "\tfilemode = true\n"
                              + "\tbare = " + std::string{ bare ? "true" : "false" }
                              + std::string{ bare ? "" : "\n\tlogallrefupdates = true\n" };

    std::ofstream config(gitDir / "config");
    if (!config.is_open())
    {
        throw std::runtime_error("Failed to create config file");
        return false;
    }
    config << configContent;
    config.close();

    return true;
}

auto Repository::getRemoteUrls() const -> std::unordered_set<std::string>
{
    auto commandExecutor = GitCommandExecutorUnix();
    auto remote_output = commandExecutor.execute(path.string(), "remote", "get-url", "--all", "origin");

    if (remote_output.return_code != 0)
    {
        throw std::runtime_error("Failed to get remote urls");
    }
    if (remote_output.stdout.empty())
    {
        return std::unordered_set<std::string>();
    }

    std::istringstream iss(remote_output.stdout);
    std::unordered_set<std::string> urls;
    std::string line;
    while (std::getline(iss, line))
    {
        urls.insert(line);
    }

    return urls;
}

auto Repository::getConfig() const -> std::vector<GitConfigEntry>
{
    auto commandExecutor = GitCommandExecutorUnix();
    auto config_output = commandExecutor.execute(path.string(), "config", "--list", "--local");

    if (config_output.return_code != 0)
    {
        throw std::runtime_error("Failed to get config");
    }
    if (config_output.stdout.empty())
    {
        return std::vector<GitConfigEntry>();
    }

    std::istringstream iss(config_output.stdout);
    std::vector<GitConfigEntry> config;

    std::string line;
    while (std::getline(iss, line))
    {
        auto delimiterPos = line.find('=');
        if (delimiterPos == std::string::npos)
        {
            config.emplace_back(line, "");
            continue;
        }
        config.emplace_back(line.substr(0, delimiterPos), line.substr(delimiterPos + 1));
    }

    return config;
}

auto Repository::getDescription() const -> std::string
{
    auto topLevelRepoPathString = getTopLevelPathAsString();
    std::filesystem::path descriptionPath;

    if (auto descriptionPathInGit = std::filesystem::path{ std::string(topLevelRepoPathString + "/.git/description") };
        std::filesystem::exists(descriptionPathInGit))
    {
        descriptionPath = std::filesystem::path{ descriptionPathInGit };
    }
    else if (auto descriptionPathInBareGit = std::filesystem::path{ (topLevelRepoPathString + "/description") };
             std::filesystem::exists(descriptionPathInBareGit))
    {
        descriptionPath = std::filesystem::path{ descriptionPathInBareGit };
    }
    else
    {
        throw std::runtime_error("Failed to get description");
    }

    std::ifstream descriptionFile(descriptionPath);
    if (!descriptionFile.is_open())
    {
        throw std::runtime_error("Failed to open description file");
    }

    std::stringstream buffer;
    buffer << descriptionFile.rdbuf();
    auto description = buffer.str();

    if (auto unnamedPos = description.find("Unnamed repository");
        unnamedPos == 0)
    {
        return std::string();
    }

    return description;
}

} // namespace CppGit

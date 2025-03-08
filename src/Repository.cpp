#include "Repository.hpp"

#include "Branches.hpp"
#include "CherryPick.hpp"
#include "Commits.hpp"
#include "CommitsHistory.hpp"
#include "Diff.hpp"
#include "Error.hpp"
#include "Index.hpp"
#include "Merge.hpp"
#include "Rebase.hpp"
#include "Reset.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

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

auto Repository::Merge() const -> CppGit::Merge
{
    return CppGit::Merge(*this);
}

auto Repository::CherryPick() const -> CppGit::CherryPick
{
    return CppGit::CherryPick(*this);
}

auto Repository::Rebase() const -> CppGit::Rebase
{
    return CppGit::Rebase(*this);
}

auto Repository::Reset() const -> CppGit::Reset
{
    return CppGit::Reset(*this);
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
    auto output = executeGitCommand("rev-parse", "--show-toplevel");

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to get top level path");
    }

    if (output.stdout.find('\n') != std::string::npos)
    {
        output.stdout.replace(output.stdout.find('\n'), 1, "");
    }

    return { output.stdout };
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

    if (auto output = executeGitCommand("rev-parse", "--is-inside-work-tree"); output.return_code != 0)
    {
        return false;
    }

    return true;
}

auto Repository::clone(const std::string& url, const std::filesystem::path& path) -> Repository
{
    auto repository = Repository(path);
    if (repository.clone(url) != Error::NO_ERROR)
    {
        throw std::runtime_error("Failed to clone repository");
    }
    return repository;
}

auto Repository::clone(const std::string& url) const -> Error
{
    if (path.empty())
    {
        return Error::CLONE_NO_PATH_GIVEN;
    }

    if (!std::filesystem::exists(path))
    {
        try
        {
            std::filesystem::create_directories(path);
        }
        catch (const std::filesystem::filesystem_error&)
        {
            return Error::FAILED_TO_CREATE_DIRECTORIES;
        }
    }
    else
    {
        if (!std::filesystem::is_directory(path))
        {
            return Error::PATH_IS_NOT_A_DIRECTORY;
        }
        if (!std::filesystem::is_empty(path))
        {
            return Error::PATH_DIR_IS_NOT_EMPTY;
        }
    }

    if (auto outout = executeGitCommand("clone", url, path.string()); outout.return_code != 0)
    {
        return Error::CLONE_FAILED;
    }

    return Error::NO_ERROR;
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
        }

        if (bare)
        {
            if (!std::filesystem::is_empty(path))
            {
                throw std::runtime_error("Directory is not empty");
            }
        }
        else
        {
            if (std::filesystem::exists(gitDir))
            {
                throw std::runtime_error("Git directory already exists");
            }
        }
    }

    if (!std::filesystem::create_directories(gitDir))
    {
        throw std::runtime_error("Failed to create git directory");
    }

    if (auto objectsDir = gitDir / "objects"; !std::filesystem::create_directories(objectsDir))
    {
        throw std::runtime_error("Failed to create objects directory");
    }

    auto refsDir = gitDir / "refs";
    if (!std::filesystem::create_directories(refsDir))
    {
        throw std::runtime_error("Failed to create refs directory");
    }

    if (!std::filesystem::create_directory(refsDir / "heads")
        || !std::filesystem::create_directory(refsDir / "tags"))
    {
        throw std::runtime_error("Failed to create heads or tags directory");
    }

    std::ofstream headFile(gitDir / "HEAD");
    if (!headFile.is_open())
    {
        throw std::runtime_error("Failed to create HEAD file");
    }
    headFile << "ref: refs/heads/" << mainBranchName;
    headFile.close();

    const auto configContent = std::string{ "[core]\n" }
                             + "\trepositoryformatversion = 0\n"
                             + "\tfilemode = true\n"
                             + "\tbare = " + std::string{ bare ? "true" : "false" }
                             + std::string{ bare ? "" : "\n\tlogallrefupdates = true\n" };

    std::ofstream config(gitDir / "config");
    if (!config.is_open())
    {
        throw std::runtime_error("Failed to create config file");
    }
    config << configContent;
    config.close();

    return true;
}

auto Repository::getRemoteUrls() const -> std::unordered_set<std::string>
{
    auto remote_output = executeGitCommand("remote", "get-url", "--all", "origin");

    if (remote_output.return_code != 0)
    {
        throw std::runtime_error("Failed to get remote urls");
    }
    if (remote_output.stdout.empty())
    {
        return {};
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
    auto config_output = executeGitCommand("config", "--list", "--local");

    if (config_output.return_code != 0)
    {
        throw std::runtime_error("Failed to get config");
    }
    if (config_output.stdout.empty())
    {
        return {};
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
        return {};
    }

    return description;
}

} // namespace CppGit

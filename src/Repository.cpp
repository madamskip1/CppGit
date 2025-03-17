#include "CppGit/Repository.hpp"

#include "CppGit/Branches.hpp"
#include "CppGit/CherryPick.hpp"
#include "CppGit/Commits.hpp"
#include "CppGit/CommitsLog.hpp"
#include "CppGit/Diff.hpp"
#include "CppGit/Index.hpp"
#include "CppGit/Merge.hpp"
#include "CppGit/Rebase.hpp"
#include "CppGit/Reset.hpp"
#include "CppGit/_details/FileUtility.hpp"

#include <algorithm>
#include <filesystem>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace CppGit {

Repository::Repository(std::filesystem::path path)
    : path(std::move(path))
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

auto Repository::CommitsLog() const -> CppGit::CommitsLog
{
    return CppGit::CommitsLog(*this);
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

    if (output.stdout.contains('\n'))
    {
        output.stdout.replace(output.stdout.find('\n'), 1, "");
    }

    return { output.stdout };
}

auto Repository::getGitDirectoryPath() const -> std::filesystem::path
{
    return getTopLevelPath() / ".git";
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

    auto canonicalGitDirStr = canonicalGitDir.string();
    const auto canonicalArgPathStr = canonicalArgPath.string();

    if (canonicalArgPathStr.size() < canonicalGitDirStr.size())
    {
        return false;
    }

    return std::ranges::mismatch(std::move(canonicalGitDirStr), canonicalArgPathStr).in1 == canonicalArgPathStr.end();
}

auto Repository::isValidGitRepository() const -> bool
{
    if (!std::filesystem::exists(path))
    {
        return false;
    }
    auto output = executeGitCommand("rev-parse", "--is-inside-work-tree");
    return output.return_code == 0;
}

auto Repository::initRepository(const bool bare, const std::string_view mainBranchName) const -> void
{
    std::filesystem::path gitDir = path;

    if (!bare)
    {
        gitDir /= ".git";
    }

    std::filesystem::create_directories(gitDir);

    const auto objectsDir = gitDir / "objects";
    std::filesystem::create_directories(objectsDir);

    auto refsDir = gitDir / "refs";
    std::filesystem::create_directories(refsDir);
    std::filesystem::create_directory(refsDir / "heads");
    std::filesystem::create_directory(refsDir / "tags");

    const auto headFileContent = std::string{ "ref: refs/heads/" } + std::string{ mainBranchName };
    _details::FileUtility::createOrOverwriteFile(gitDir / "HEAD", headFileContent);

    const auto configContent = std::string{ "[core]\n" }
                             + "\trepositoryformatversion = 0\n"
                             + "\tfilemode = true\n"
                             + "\tbare = " + std::string{ bare ? "true" : "false" }
                             + std::string{ bare ? "" : "\n\tlogallrefupdates = true\n" };

    _details::FileUtility::createOrOverwriteFile(gitDir / "config", configContent);
}

auto Repository::getRemoteUrls() const -> std::unordered_set<std::string>
{
    auto remote_output = executeGitCommand("remote", "get-url", "--all", "origin");

    if (remote_output.stdout.empty())
    {
        return {};
    }

    std::istringstream iss(std::move(remote_output.stdout));
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

    if (config_output.stdout.empty())
    {
        return {};
    }

    std::istringstream iss(std::move(config_output.stdout));
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
    const auto topLevelRepoPathString = getTopLevelPathAsString();
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

    auto description = _details::FileUtility::readFile(descriptionPath);

    if (auto unnamedPos = description.find("Unnamed repository");
        unnamedPos == 0)
    {
        return {};
    }

    return description;
}

} // namespace CppGit

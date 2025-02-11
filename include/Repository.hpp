#pragma once


#include "Error.hpp"
#include "_details/GitCommandExecutor/GitCommandExecutorUnix.hpp"
#include "_details/GitCommandExecutor/GitCommandOutput.hpp"

#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

namespace CppGit {

class Branches;       // forward-declaration
class Index;          // forward-declaration
class Commits;        // forward-declaration
class CommitsHistory; // forward-declaration
class Diff;           // forward-declaration
class Merge;          // forward-declaration
class CherryPick;     // forward-declaration
class Rebase;         // forward-declaration

using GitConfigEntry = std::pair<std::string, std::string>;
class Repository
{
public:
    /// @param path Path to the repository
    explicit Repository(const std::filesystem::path& path);
    Repository() = delete;

    /// @brief Execute git command
    /// @tparam Args Command arguments types
    /// @param environmentVariables Environment variables to set before executing the command
    /// @param cmd Command to execute
    /// @param args Command arguments
    /// @return Command output
    template <typename... Args>
    auto executeGitCommand(const std::vector<std::string>& environmentVariables, const std::string_view cmd, Args&&... args) const -> GitCommandOutput
    {
        auto commandExecutor = GitCommandExecutorUnix();
        return commandExecutor.execute(environmentVariables, path.string(), cmd, std::forward<Args>(args)...);
    }

    /// @brief Execute git command
    /// @tparam Args Command arguments types
    /// @param cmd Command to execute
    /// @param args Command arguments
    /// @return Command output
    template <typename... Args>
    auto executeGitCommand(const std::string_view cmd, Args&&... args) const -> GitCommandOutput
    {
        return executeGitCommand(std::vector<std::string>{}, cmd, std::forward<Args>(args)...);
    }

    /// @brief Return branches object with current repository
    /// @return Branches object
    auto Branches() const -> CppGit::Branches;

    /// @brief Return index object with current repository
    /// @return Index object
    auto Index() const -> CppGit::Index;

    /// @brief Return commits object with current repository
    /// @return Commits object
    auto Commits() const -> CppGit::Commits;

    /// @brief Return commits history object with current repository
    /// @return CommitsHistory object
    auto CommitsHistory() const -> CppGit::CommitsHistory;

    /// @brief Return diff object with current repository
    /// @return Diff object
    auto Diff() const -> CppGit::Diff;

    /// @brief Return merge object with current repository
    /// @return Merge object
    auto Merge() const -> CppGit::Merge;

    /// @brief Return cherry-pick object with current repository
    /// @return CherryPick object
    auto CherryPick() const -> CppGit::CherryPick;

    /// @brief Return rebase object with current repository
    /// @return Rebase object
    auto Rebase() const -> CppGit::Rebase;

    /// @brief Get repository path as string
    /// @return Repository path as string
    auto getPathAsString() const -> std::string;

    /// @brief Get repository path as std::filesystem path
    /// @return Repository path as std::filesystem path
    auto getPath() const -> std::filesystem::path;

    /// @brief Get top level repository path as string
    /// @return Top level repository path as string
    auto getTopLevelPathAsString() const -> std::string;

    /// @brief Get top level repository path as std::filesystem path
    /// @return Top level repository path as std::filesystem path
    auto getTopLevelPath() const -> std::filesystem::path;

    /// @brief Get git directory path as string
    /// @return Git directory path as string
    auto getGitDirectoryPath() const -> std::filesystem::path;

    /// @brief Transform relative path to absolute path
    /// @param relativePath Relative path
    auto getAbsoluteFromRelativePath(const std::filesystem::path& relativePath) const -> std::filesystem::path;

    /// @brief Transform absolute path to relative path
    /// @param absolutePath Absolute path
    auto getRelativeFromAbsolutePath(const std::filesystem::path& absolutePath) const -> std::filesystem::path;

    /// @brief Check whether the path is in the git directory
    /// @param path Path to check
    /// @return True if the path is in the git directory, false otherwise
    auto isPathInGitDirectory(const std::filesystem::path& path) const -> bool;

    /// @brief Check whether the path is a valid git repository
    /// @return True if the path is a valud git repository, false otherwise
    auto isValidGitRepository() const -> bool;

    /// @brief Clone repository
    /// @param url Repository URL
    /// @param path Path to clone repository to
    /// @return Repository object with cloned repository
    static auto clone(const std::string& url, const std::filesystem::path& path) -> Repository;

    /// @brief Clone repository
    /// @param url Repository URL
    /// @return Error code if cloning failed, otherwise no error
    auto clone(const std::string& url) const -> Error;

    /// @brief Initialize repository
    /// @param bare Whether to initialize bare repository
    /// @param mainBranchName Main branch name (default: "main")
    /// @return True if repository was initialized, false otherwise
    auto initRepository(bool bare = false, std::string_view mainBranchName = "main") const -> bool;

    /// @brief Get remote URL
    /// @return Remote URL
    auto getRemoteUrls() const -> std::unordered_set<std::string>;

    /// @brief Get repository's config
    /// @return Repository's config
    auto getConfig() const -> std::vector<GitConfigEntry>;

    /// @brief Get repository's description
    /// @return Repository's description
    auto getDescription() const -> std::string;

private:
    std::filesystem::path path;
};

using Repo = Repository;

} // namespace CppGit

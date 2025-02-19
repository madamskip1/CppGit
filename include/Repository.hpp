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
class Reset;          // forward-declaration

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
    [[nodiscard]] auto Branches() const -> CppGit::Branches;

    /// @brief Return index object with current repository
    /// @return Index object
    [[nodiscard]] auto Index() const -> CppGit::Index;

    /// @brief Return commits object with current repository
    /// @return Commits object
    [[nodiscard]] auto Commits() const -> CppGit::Commits;

    /// @brief Return commits history object with current repository
    /// @return CommitsHistory object
    [[nodiscard]] auto CommitsHistory() const -> CppGit::CommitsHistory;

    /// @brief Return diff object with current repository
    /// @return Diff object
    [[nodiscard]] auto Diff() const -> CppGit::Diff;

    /// @brief Return merge object with current repository
    /// @return Merge object
    [[nodiscard]] auto Merge() const -> CppGit::Merge;

    /// @brief Return cherry-pick object with current repository
    /// @return CherryPick object
    [[nodiscard]] auto CherryPick() const -> CppGit::CherryPick;

    /// @brief Return rebase object with current repository
    /// @return Rebase object
    [[nodiscard]] auto Rebase() const -> CppGit::Rebase;

    /// @brief Return reset object with current repository
    /// @return Reset object
    [[nodiscard]] auto Reset() const -> CppGit::Reset;

    /// @brief Get repository path as string
    /// @return Repository path as string
    [[nodiscard]] auto getPathAsString() const -> std::string;

    /// @brief Get repository path as std::filesystem path
    /// @return Repository path as std::filesystem path
    [[nodiscard]] auto getPath() const -> std::filesystem::path;

    /// @brief Get top level repository path as string
    /// @return Top level repository path as string
    [[nodiscard]] auto getTopLevelPathAsString() const -> std::string;

    /// @brief Get top level repository path as std::filesystem path
    /// @return Top level repository path as std::filesystem path
    [[nodiscard]] auto getTopLevelPath() const -> std::filesystem::path;

    /// @brief Get git directory path as string
    /// @return Git directory path as string
    [[nodiscard]] auto getGitDirectoryPath() const -> std::filesystem::path;

    /// @brief Transform relative path to absolute path
    /// @param relativePath Relative path
    [[nodiscard]] auto getAbsoluteFromRelativePath(const std::filesystem::path& relativePath) const -> std::filesystem::path;

    /// @brief Transform absolute path to relative path
    /// @param absolutePath Absolute path
    [[nodiscard]] auto getRelativeFromAbsolutePath(const std::filesystem::path& absolutePath) const -> std::filesystem::path;

    /// @brief Check whether the path is in the git directory
    /// @param path Path to check
    /// @return True if the path is in the git directory, false otherwise
    [[nodiscard]] auto isPathInGitDirectory(const std::filesystem::path& path) const -> bool;

    /// @brief Check whether the path is a valid git repository
    /// @return True if the path is a valud git repository, false otherwise
    [[nodiscard]] auto isValidGitRepository() const -> bool;

    /// @brief Clone repository
    /// @param url Repository URL
    /// @param path Path to clone repository to
    /// @return Repository object with cloned repository
    [[nodiscard]] static auto clone(const std::string& url, const std::filesystem::path& path) -> Repository;

    /// @brief Clone repository
    /// @param url Repository URL
    /// @return Error code if cloning failed, otherwise no error
    auto clone(const std::string& url) const -> Error;

    /// @brief Initialize repository
    /// @param bare Whether to initialize bare repository
    /// @param mainBranchName Main branch name (default: "main")
    /// @return True if repository was initialized, false otherwise
    auto initRepository(const bool bare = false, const std::string_view mainBranchName = "main") const -> bool;

    /// @brief Get remote URL
    /// @return Remote URL
    [[nodiscard]] auto getRemoteUrls() const -> std::unordered_set<std::string>;

    /// @brief Get repository's config
    /// @return Repository's config
    [[nodiscard]] auto getConfig() const -> std::vector<GitConfigEntry>;

    /// @brief Get repository's description
    /// @return Repository's description
    [[nodiscard]] auto getDescription() const -> std::string;

private:
    std::filesystem::path path;
};

using Repo = Repository;

} // namespace CppGit

#pragma once

#include "_details/GitCommandExecutor/GitCommandExecutorUnix.hpp"
#include "_details/GitCommandExecutor/GitCommandOutput.hpp"

#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

namespace CppGit {

class BranchesManager;   // forward-declaration
class IndexManager;      // forward-declaration
class CommitsManager;    // forward-declaration
class CommitsLogManager; // forward-declaration
class DiffGenerator;     // forward-declaration
class Merger;            // forward-declaration
class CherryPicker;      // forward-declaration
class Rebaser;           // forward-declaration
class Resetter;          // forward-declaration

using GitConfigEntry = std::pair<std::string, std::string>;

/// @brief Represents a git repository
class Repository
{
public:
    /// @param path Path to the repository
    explicit Repository(std::filesystem::path path);
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

    /// @brief Return branches manager object with current repository
    /// @return BranchesManager object
    [[nodiscard]] auto BranchesManager() const -> CppGit::BranchesManager;

    /// @brief Return index manager object with current repository
    /// @return IndexManager object
    [[nodiscard]] auto IndexManager() const -> CppGit::IndexManager;

    /// @brief Return commits manager object with current repository
    /// @return CommitsManager object
    [[nodiscard]] auto CommitsManager() const -> CppGit::CommitsManager;

    /// @brief Return commits log manager object with current repository
    /// @return CommitsLogManager object
    [[nodiscard]] auto CommitsLogManager() const -> CppGit::CommitsLogManager;

    /// @brief Return diff generator object with current repository
    /// @return DiffGenerator object
    [[nodiscard]] auto DiffGenerator() const -> CppGit::DiffGenerator;

    /// @brief Return merger object with current repository
    /// @return Merger object
    [[nodiscard]] auto Merger() const -> CppGit::Merger;

    /// @brief Return cherry-picker object with current repository
    /// @return CherryPicker object
    [[nodiscard]] auto CherryPicker() const -> CppGit::CherryPicker;

    /// @brief Return rebaser object with current repository
    /// @return Rebaser object
    [[nodiscard]] auto Rebaser() const -> CppGit::Rebaser;

    /// @brief Return reseter object with current repository
    /// @return Reseter object
    [[nodiscard]] auto Resetter() const -> CppGit::Resetter;

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

    /// @brief Initialize repository
    /// @param bare Whether to initialize bare repository
    /// @param mainBranchName Main branch name (default: "main")
    auto initRepository(const bool bare = false, const std::string_view mainBranchName = "main") const -> void;

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

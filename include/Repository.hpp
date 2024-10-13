#pragma once

#include "Branches.hpp"
#include "Commits.hpp"
#include "CommitsHistory.hpp"
#include "Diff.hpp"
#include "ErrorCodes.hpp"
#include "GitCommandExecutor/GitCommandExecutorUnix.hpp"
#include "GitCommandExecutor/GitCommandOutput.hpp"
#include "Index.hpp"

#include <filesystem>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace CppGit {

using GitConfigEntry = std::pair<std::string, std::string>;
class Repository
{
public:
    explicit Repository(const std::filesystem::path& path);
    Repository() = delete;

    template <typename... Args>
    auto executeGitCommand(const std::string_view cmd, Args... args) const -> GitCommandOutput
    {
        auto commandExecutor = GitCommandExecutorUnix();
        return commandExecutor.execute(path.string(), cmd, args...);
    }

    auto Branches() const -> CppGit::Branches;
    auto Index() const -> CppGit::Index;
    auto Commits() const -> CppGit::Commits;
    auto CommitsHistory() const -> CppGit::CommitsHistory;
    auto Diff() const -> CppGit::Diff;

    auto getPathAsString() const -> std::string;
    auto getPath() const -> std::filesystem::path;
    auto getTopLevelPathAsString() const -> std::string;
    auto getTopLevelPath() const -> std::filesystem::path;
    auto getGitDirectoryPath() const -> std::filesystem::path;

    auto getAbsoluteFromRelativePath(const std::filesystem::path& relativePath) const -> std::filesystem::path;
    auto getRelativeFromAbsolutePath(const std::filesystem::path& absolutePath) const -> std::filesystem::path;

    auto isPathInGitDirectory(const std::filesystem::path& path) const -> bool;

    auto isValidGitRepository() const -> bool;

    static auto clone(const std::string& url, const std::filesystem::path& path) -> Repository;
    auto clone(const std::string& url) const -> ErrorCode;

    auto initRepository(bool bare = false, std::string_view mainBranchName = "main") const -> bool;

    auto getRemoteUrls() const -> std::unordered_set<std::string>;
    auto getConfig() const -> std::vector<GitConfigEntry>;
    auto getDescription() const -> std::string;

private:
    std::filesystem::path path;
};

using Repo = Repository;

} // namespace CppGit

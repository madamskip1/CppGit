#pragma once

#include "Branches.hpp"
#include "ErrorCodes.hpp"
#include "GitCommandExecutor/GitCommandExecutorUnix.hpp"
#include "GitCommandExecutor/GitCommandOutput.hpp"

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
    GitCommandOutput executeGitCommand(const std::string_view cmd, Args... args) const
    {
        auto commandExecutor = GitCommandExecutorUnix();
        return commandExecutor.execute(path.string(), cmd, args...);
    }

    CppGit::Branches Branches() const;

    std::string getPathAsString() const;
    std::filesystem::path getPath() const;
    std::string getTopLevelPathAsString() const;
    std::filesystem::path getTopLevelPath() const;
    std::filesystem::path getGitDirectoryPath() const;

    std::filesystem::path getAbsoluteFromRelativePath(const std::filesystem::path& relativePath) const;
    std::filesystem::path getRelativeFromAbsolutePath(const std::filesystem::path& absolutePath) const;

    bool isPathInGitDirectory(const std::filesystem::path& path) const;

    bool isValidGitRepository() const;

    static Repository clone(const std::string& url, const std::filesystem::path& path);
    ErrorCode clone(const std::string& url) const;

    bool initRepository(bool bare = false, std::string_view mainBranchName = "main") const;

    std::unordered_set<std::string> getRemoteUrls() const;
    std::vector<GitConfigEntry> getConfig() const;
    std::string getDescription() const;

private:
    std::filesystem::path path;
};

using Repo = Repository;

} // namespace CppGit

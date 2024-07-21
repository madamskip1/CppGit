#pragma once

#include <string>
#include <filesystem>
#include <vector>
#include <utility>
#include <unordered_set>
#include "GitCommandExecutor/GitCommandOutput.hpp"
#include "GitCommandExecutor/GitCommandExecutorUnix.hpp"
#include "ErrorCodes.hpp"
#include "Branches.hpp"

namespace CppGit
{
    using GitConfigEntry = std::pair<std::string, std::string>;
    class Repository
    {
    public:
        explicit Repository(const std::filesystem::path& path);
        Repository() = delete;

        template<typename... Args>
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
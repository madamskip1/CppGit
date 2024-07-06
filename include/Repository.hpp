#pragma once

#include <string>
#include <filesystem>
#include <vector>
#include <utility>
#include <unordered_set>
#include "GitCommandExecutor.hpp"
#include "ErrorCodes.hpp"

namespace CppGit
{
    constexpr const char* const CHECK_IF_GIT_REPOSTITORY_CMD = "rev-parse --is-inside-work-tree";

    using GitConfigEntry = std::pair<std::string, std::string>;
    class Repository
    {
    public:
        Repository(const std::filesystem::path& path);

        GitCommandOutput executeGitCommand(std::string_view cmd) const;

        std::string getPathAsString() const;
        std::filesystem::path getPath() const;
        std::string getTopLevelPathAsString() const;
        std::filesystem::path getTopLevelPath() const;

        bool isValidGitRepository() const;

        static Repository clone(const std::string& url, const std::filesystem::path& path);
        ErrorCode clone(const std::string& url) const;

        std::unordered_set<std::string> getRemoteUrls() const;
        std::vector<GitConfigEntry> getConfig() const;
        std::string getDescription() const;

    private:
        std::filesystem::path path;
    };

    using Repo = Repository;
} // namespace CppGit
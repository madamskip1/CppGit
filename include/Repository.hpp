#pragma once

#include <string>
#include <filesystem>
#include <unordered_set>
#include "GitCommandExecutor.hpp"
#include "ErrorCodes.hpp"

namespace CppGit
{
    constexpr const char* const CHECK_IF_GIT_REPOSTITORY_CMD = "rev-parse --is-inside-work-tree";

    class Repository
    {
    public:
        Repository(const std::filesystem::path& path);

        std::string getPathAsString() const;
        std::filesystem::path getPath() const;

        bool isValidGitRepository() const;

        static Repository clone(const std::string& url, const std::filesystem::path& path);
        ErrorCode clone(const std::string& url) const;

        std::unordered_set<std::string> getRemoteUrls() const;

    private:
        std::filesystem::path path;
    };

    using Repo = Repository;
} // namespace CppGit
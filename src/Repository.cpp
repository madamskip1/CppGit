#include "Repository.hpp"

namespace CppGit
{
    Repository::Repository(const std::filesystem::path& path) : path(path)
    {}
    
    std::string Repository::getPathAsString() const
    {
        return path.string();
    }

    std::filesystem::path Repository::getPath() const
    {
        return path;
    }

    bool Repository::isValidGitRepository() const
    {
        if (!std::filesystem::exists(path))
        {
            return false;
        }

        if (GitCommandExecutor::exec(path.string(), CHECK_IF_GIT_REPOSTITORY_CMD).return_code != 0)
        {
            return false;
        }

        return true;
    }
} // namespace CppGit
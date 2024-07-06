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

    Repository Repository::clone(const std::string &url, const std::filesystem::path &path)
    {
        auto repository = Repository(path);
        if (repository.clone(url) != ErrorCode::NO_ERROR)
        {
            throw std::runtime_error("Failed to clone repository");
        }
        return repository;
    }

    ErrorCode Repository::clone(const std::string &url) const
    {
        if (path.empty())
        {
            return ErrorCode::GIT_CLONE_NO_PATH_GIVEN;
        }
        
        if (!std::filesystem::exists(path))
        {
            try
            {
                std::filesystem::create_directories(path);
            }
            catch (const std::filesystem::filesystem_error& e)
            {
                return ErrorCode::GIT_CLONE_FAILED_TO_CREATE_DIRECTORIES;
            }
        }
        else 
        {
            if (!std::filesystem::is_directory(path))
            {
                return ErrorCode::GIT_CLONE_PATH_IS_NOT_A_DIRECTORY;
            }
            if (!std::filesystem::is_empty(path))
            {
                return ErrorCode::GIT_CLONE_PATH_DIR_IS_NOT_EMPTY;
            }
        }

        auto clone_output = GitCommandExecutor::exec("clone " + url + " " + path.string());
        
        if (clone_output.return_code != 0)
        {
            return ErrorCode::GIT_CLONE_FAILED;
        }

        return ErrorCode::NO_ERROR;
    }
} // namespace CppGit
#include "Repository.hpp"
#include <sstream>


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

    std::unordered_set<std::string> Repository::getRemoteUrls() const
    {
        auto remote_output = GitCommandExecutor::exec("remote get-url --all origin", path.string());

        if (remote_output.return_code != 0)
        {
            throw std::runtime_error("Failed to get remote urls");
        }
        if (remote_output.output.empty())
        {
            return std::unordered_set<std::string>();
        }

        std::istringstream iss(remote_output.output);
        std::unordered_set<std::string> urls;
        std::string line;
        while (std::getline(iss, line))
        {
            urls.insert(line);
        }

        return urls;
    }
    
    std::vector<GitConfigEntry> Repository::getConfig() const
    {
        auto config_output = GitCommandExecutor::exec("config --list --local", path.string());

        if (config_output.return_code != 0)
        {
            throw std::runtime_error("Failed to get config");
        }
        if (config_output.output.empty())
        {
            return std::vector<GitConfigEntry>();
        }

        std::istringstream iss(config_output.output);
        std::vector<GitConfigEntry> config;

        std::string line;
        while (std::getline(iss, line))
        {
            auto delimiterPos = line.find('=');
            if (delimiterPos == std::string::npos)
            {
                config.emplace_back(line, "");
                continue;
            }
            config.emplace_back(std::make_pair(line.substr(0, delimiterPos), line.substr(delimiterPos + 1)));
        }

        return config;
    }
} // namespace CppGit
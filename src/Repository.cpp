#include "Repository.hpp"
#include <sstream>
#include <fstream>
#include "GitCommandExecutor/GitCommandExecutorUnix.hpp"
#include <iostream>

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

    std::string Repository::getTopLevelPathAsString() const
    {
        return getTopLevelPath().string();
    }

    std::filesystem::path Repository::getTopLevelPath() const
    {
        auto commandExecutor = GitCommandExecutorUnix();

        auto output = commandExecutor.execute(path.string(), "rev-parse", "--show-toplevel");

        if (output.return_code != 0)
        {
            throw std::runtime_error("Failed to get top level path");
        }
        std::string x;
        if (output.stdout.find("\n") != std::string::npos)
        {
            output.stdout.replace(output.stdout.find('\n'), 1, "");
        }

        return std::filesystem::path(output.stdout);
    }

    bool Repository::isValidGitRepository() const
    {
        if (!std::filesystem::exists(path))
        {
            return false;
        }
        std::cout << "istnieje folder " << std::endl;
        if (auto commandExecutor = GitCommandExecutorUnix(); commandExecutor.execute(path.string(), "rev-parse", "--is-inside-work-tree").return_code != 0)
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

        auto commandExecutor = GitCommandExecutorUnix();

        if (auto clone_output = commandExecutor.execute(path.string(), "clone", url, path.string()); clone_output.return_code != 0)
        {
            return ErrorCode::GIT_CLONE_FAILED;
        }

        return ErrorCode::NO_ERROR;
    }

    bool Repository::initRepository(bool bare, std::string_view mainBranchName) const
    {
        std::filesystem::path gitDir = path;
        if (!bare)
        {
            gitDir /= ".git";
        }
        
        if (std::filesystem::exists(path))
        {
            if (!std::filesystem::is_directory(path))
            {
                throw std::runtime_error("Path is not a directory");
                return false;
            }

            if (bare)
            {
                if (!std::filesystem::is_empty(path))
                {
                    throw std::runtime_error("Directory is not empty");
                    return false;
                }
            }
            else
            {
                if (std::filesystem::exists(gitDir))
                {
                    throw std::runtime_error("Git directory already exists");
                    return false;
                }
            }
        }

        if (!std::filesystem::create_directories(gitDir))
        {
            throw std::runtime_error("Failed to create git directory");
            return false;
        }

        if (auto objectsDir = gitDir / "objects"; !std::filesystem::create_directories(objectsDir))
        {
            throw std::runtime_error("Failed to create objects directory");
            return false;
        }

        auto refsDir = gitDir / "refs";
        if (!std::filesystem::create_directories(refsDir))
        {
            throw std::runtime_error("Failed to create refs directory");
            return false;
        }

        if (!std::filesystem::create_directory(refsDir / "heads")
            || !std::filesystem::create_directory(refsDir / "tags"))
        {
            throw std::runtime_error("Failed to create heads or tags directory");
            return false;
        }

        std::ofstream headFile(gitDir / "HEAD");
        if (!headFile.is_open())
        {
            throw std::runtime_error("Failed to create HEAD file");
            return false;
        }
        headFile << "ref: refs/heads/" << mainBranchName << std::endl;
        headFile.close();

        std::string configContent = std::string{ "[core]\n" }
                                    +  "\trepositoryformatversion = 0\n"
                                    + "\tfilemode = true\n"
                                    + "\tbare = " + std::string { bare ? "true" :  "false" } + "\n"
                                    + std::string{ bare ? "" : "\tlogallrefupdates = true\n" };

        std::ofstream config(gitDir / "config");
        if (!config.is_open())
        {
            throw std::runtime_error("Failed to create config file");
            return false;
        }
        config << configContent;
        config.close();

        return false;
    }

    std::unordered_set<std::string> Repository::getRemoteUrls() const
    {
        auto commandExecutor = GitCommandExecutorUnix();
        auto remote_output = commandExecutor.execute(path.string(), "remote", "get-url", "--all", "origin");

        if (remote_output.return_code != 0)
        {
            throw std::runtime_error("Failed to get remote urls");
        }
        if (remote_output.stdout.empty())
        {
            return std::unordered_set<std::string>();
        }

        std::istringstream iss(remote_output.stdout);
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
        auto commandExecutor = GitCommandExecutorUnix();
        auto config_output = commandExecutor.execute(path.string(), "config", "--list", "--local");

        if (config_output.return_code != 0)
        {
            throw std::runtime_error("Failed to get config");
        }
        if (config_output.stdout.empty())
        {
            return std::vector<GitConfigEntry>();
        }

        std::istringstream iss(config_output.stdout);
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

    std::string Repository::getDescription() const
    {
        auto topLevelRepoPathString = getTopLevelPathAsString();
        std::filesystem::path descriptionPath;

        if (auto descriptionPathInGit = std::filesystem::path{ std::string(topLevelRepoPathString + "/.git/description") };
            std::filesystem::exists(descriptionPathInGit))
        {
            descriptionPath = std::filesystem::path { descriptionPathInGit };
        }
        else if (auto descriptionPathInBareGit = std::filesystem::path{ (topLevelRepoPathString + "/description") };
            std::filesystem::exists(descriptionPathInBareGit))
        {
            descriptionPath = std::filesystem::path { descriptionPathInBareGit };
        }
        else
        {
            throw std::runtime_error("Failed to get description");
        }
        
        std::ifstream descriptionFile(descriptionPath);
        if (!descriptionFile.is_open())
        {
            throw std::runtime_error("Failed to open description file");
        }

        std::stringstream buffer;
        buffer << descriptionFile.rdbuf();
        auto description = buffer.str();

        if (auto unnamedPos = description.find("Unnamed repository");
            unnamedPos != std::string::npos && unnamedPos == 0)
        {
            return std::string();
        }

        return description;
    }
} // namespace CppGit
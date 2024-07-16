#include <iostream>
#include "GitCommandExecutor/GitCommandExecutorUnix.hpp"
#include "Repository.hpp"
#include "Commit.hpp"

int main()
{
    auto commandExecutor = CppGit::GitCommandExecutorUnix();
    auto output = commandExecutor.execute("--version");
    std::cout << "git --version: return_code: " << output.return_code << ", output: " << output.stdout << std::endl;

    output = commandExecutor.execute(".", "rev-parse --is-inside-work-tree");
    std::cout << "rev-parse --is-inside-work-tree: return_code: " << output.return_code << ", output: " << output.stdout << std::endl;


    auto repo = CppGit::Repository(".");
    std::cout << "Is valid git repository: " << repo.isValidGitRepository() << std::endl;
    
    auto urls = repo.getRemoteUrls();
    for (const auto& url : urls)
    {
        std::cout << "Remote url: " << url << std::endl;
    }

    auto config = repo.getConfig();
    for (const auto& entry : config)
    {
        std::cout << "Config entry: " << entry.first << " = " << entry.second << std::endl;
    }

    std::cout << "top level path: " << repo.getTopLevelPath() << std::endl;

    std::cout << "Description: " << repo.getDescription() << std::endl;

    std::cout << "repo git status: " << repo.executeGitCommand("status").stdout << std::endl;

    auto commit = CppGit::Commit(repo, "HEAD");
    std::cout << "Commit hash: " << commit.getHash() << std::endl;
    std::cout << "Author name: " << commit.getAuthor().name << std::endl;
    std::cout << "message: " << commit.getMessage() << std::endl;
    return 0;
}
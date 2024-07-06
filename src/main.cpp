#include <iostream>
#include "GitCommandExecutor.hpp"
#include "Repository.hpp"

int main()
{
    CppGit::GitCommandExecutor executor;
    auto output = executor.execute("--version");
    std::cout << "git --version: return_code: " << output.return_code << ", output: " << output.output << std::endl;

    output = executor.execute(".", "rev-parse --is-inside-work-tree");
    std::cout << "rev-parse --is-inside-work-tree: return_code: " << output.return_code << ", output: " << output.output << std::endl;

    auto hasGit = CppGit::GitCommandExecutor::checkIfHasGit();
    std::cout << "Has git: " << hasGit << std::endl;

    auto repo = CppGit::Repository(".");
    std::cout << "Is valid git repository: " << repo.isValidGitRepository() << std::endl;

    // auto cloned_repo = CppGit::Repository::clone("git@github.com:madamskip1/hc-FSM.git", "/home/maciej/Repozytoria/test-clone/");
    
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

    std::cout << "repo git status: " << repo.executeGitCommand("status").output << std::endl;
    return 0;
}
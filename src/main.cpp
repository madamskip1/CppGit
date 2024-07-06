#include <iostream>
#include "GitCommandExecutor.hpp"


int main()
{
    CppGit::GitCommandExecutor executor;
    auto output = executor.exec("--version");
    std::cout << "git --version: return_code: " << output.return_code << ", output: " << output.output << std::endl;

    output = executor.exec(".", "rev-parse --is-inside-work-tree");
    std::cout << "rev-parse --is-inside-work-tree: return_code: " << output.return_code << ", output: " << output.output << std::endl;

    auto hasGit = CppGit::GitCommandExecutor::checkIfHasGit();
    std::cout << "Has git: " << hasGit << std::endl;

    return 0;
}
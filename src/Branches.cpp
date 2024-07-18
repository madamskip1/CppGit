#include "Branches.hpp"
#include "Parser/BranchesParser.hpp"

namespace CppGit
{
    Branches::Branches(Repository &repo)
        : repo(repo)
    {
    }

    std::vector<Branch> Branches::getAllBranches() const
    {
        return getBranchesImpl(true, true);
    }

    std::vector<Branch> Branches::getRemoteBranches() const
    {
        return getBranchesImpl(false, true);
    }

    std::vector<Branch> Branches::getLocalBranches() const
    {
        return getBranchesImpl(true, false);
    }

    std::vector<Branch> Branches::getBranchesImpl(bool local, bool remote) const
    {
        auto argLocal = local ? "refs/heads" : "";
        auto argRemote = remote ? "refs/remotes" : "";

        auto output = repo.executeGitCommand("for-each-ref", "--format=" + std::string(BranchesParser::BRANCHES_FORMAT), argLocal, argRemote);

        if (output.return_code != 0)
        {
            throw std::runtime_error("Failed to get branches");
        }

        const auto& outputString = output.stdout;
        std::vector<Branch> branches;
        std::size_t start = 0;
        std::size_t delimiterPos = outputString.find('\n');

        while (delimiterPos != std::string::npos)
        {   
            auto line = outputString.substr(start, delimiterPos - start);
            branches.emplace_back(BranchesParser::parseBranch(line));

            start = delimiterPos + 1;
            delimiterPos = outputString.find('\n', start);
        }
        
        if (start < outputString.size())
        {
            auto line = outputString.substr(start);
            branches.emplace_back(BranchesParser::parseBranch(line));
        }

        return branches;
    }
}

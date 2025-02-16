#include "_details/CreateCommit.hpp"

#include "Repository.hpp"

#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace CppGit::_details {

CreateCommit::CreateCommit(const Repository& repo)
    : repo(repo)
{
}

auto CreateCommit::createCommit(const std::string_view message, const std::string_view description, const std::vector<std::string>& parents, const std::vector<std::string>& envp) const -> std::string
{
    auto treeHash = writeTree();
    auto commitHash = commitTree(std::move(treeHash), message, description, parents, envp);

    if (auto updateIndexOutput = repo.executeGitCommand("update-index", "--refresh", "--again", "--quiet"); updateIndexOutput.return_code != 0)
    {
        throw std::runtime_error("Failed to update index");
    }

    return commitHash;
}

auto CreateCommit::createCommit(const std::string_view message, const std::vector<std::string>& parents, const std::vector<std::string>& envp) const -> std::string
{
    return createCommit(message, "", parents, envp);
}


auto CreateCommit::writeTree() const -> std::string
{
    auto writeTreeOutput = repo.executeGitCommand("write-tree");

    if (writeTreeOutput.return_code != 0)
    {
        throw std::runtime_error("Failed to write tree");
    }

    return std::move(writeTreeOutput.stdout);
}

auto CreateCommit::commitTree(std::string&& treeHash, const std::string_view message, const std::string_view description, const std::vector<std::string>& parents, const std::vector<std::string>& envp) const -> std::string
{
    auto commitArgs = std::vector<std::string>{};
    commitArgs.reserve(1 + (2 * parents.size()) + 4); // treeHash + parents*2 + message + description

    commitArgs.push_back(std::move(treeHash));

    commitArgs.emplace_back("-m");
    commitArgs.emplace_back(message);

    if (!description.empty())
    {
        commitArgs.emplace_back("-m");
        commitArgs.emplace_back(description);
    }

    for (const auto& parent : parents)
    {
        if (!parent.empty())
        {
            commitArgs.emplace_back("-p");
            commitArgs.push_back(parent);
        }
    }

    return commitTreeImpl(commitArgs, envp);
}

auto CreateCommit::commitTreeImpl(const std::vector<std::string>& commitArgs, const std::vector<std::string>& envp) const -> std::string
{
    auto commitOutput = (envp.empty() ? repo.executeGitCommand("commit-tree", commitArgs) : repo.executeGitCommand(envp, "commit-tree", commitArgs));

    if (commitOutput.return_code != 0)
    {
        throw std::runtime_error("Failed to create commit");
    }

    return std::move(commitOutput.stdout);
}

} // namespace CppGit::_details

#include "CppGit/_details/CommitCreator.hpp"

#include "CppGit/Repository.hpp"

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace CppGit::_details {

CommitCreator::CommitCreator(const Repository& repository)
    : repository{ &repository }
{
}

auto CommitCreator::createCommit(const std::string_view message, const std::string_view description, const std::vector<std::string>& parents, const std::vector<std::string>& envp) const -> std::string
{
    auto treeHash = writeTree();
    const auto commitHash = commitTree(std::move(treeHash), message, description, parents, envp);

    repository->executeGitCommand("update-index", "--refresh", "--again", "--quiet");

    return commitHash;
}

auto CommitCreator::createCommit(const std::string_view message, const std::vector<std::string>& parents, const std::vector<std::string>& envp) const -> std::string
{
    return createCommit(message, "", parents, envp);
}

auto CommitCreator::writeTree() const -> std::string
{
    auto writeTreeOutput = repository->executeGitCommand("write-tree");

    return std::move(writeTreeOutput.stdout);
}

auto CommitCreator::commitTree(std::string&& treeHash, const std::string_view message, const std::string_view description, const std::vector<std::string>& parents, const std::vector<std::string>& envp) const -> std::string
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

    return commitTreeImpl(std::move(commitArgs), envp);
}

auto CommitCreator::commitTreeImpl(std::vector<std::string> commitArgs, const std::vector<std::string>& envp) const -> std::string
{
    auto commitOutput = (envp.empty() ? repository->executeGitCommand("commit-tree", std::move(commitArgs)) : repository->executeGitCommand(envp, "commit-tree", std::move(commitArgs)));

    return std::move(commitOutput.stdout);
}

} // namespace CppGit::_details

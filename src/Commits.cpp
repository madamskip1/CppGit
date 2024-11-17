#include "Commits.hpp"

#include "Branches.hpp"
#include "_details/GitCommandExecutor/GitCommandOutput.hpp"
#include "_details/Parser/CommitParser.hpp"

namespace CppGit {

Commits::Commits(const Repository& repo)
    : repo(repo)
{
}

auto Commits::createCommit(const std::string_view message, const std::string_view description) const -> std::string
{
    auto parent = hasAnyCommits() ? getHeadCommitHash() : std::string{};
    return createCommitImpl(message, description, { std::move(parent) });
}

auto Commits::amendCommit(const std::string_view message, const std::string_view description) const -> std::string
{
    auto headCommitHash = getHeadCommitHash();
    auto commitInfo = getCommitInfo(headCommitHash);

    auto envp = std::vector<std::string>{};
    envp.emplace_back("GIT_AUTHOR_NAME=" + commitInfo.getAuthor().name);
    envp.emplace_back("GIT_AUTHOR_EMAIL=" + commitInfo.getAuthor().email);
    envp.emplace_back("GIT_AUTHOR_DATE=" + commitInfo.getAuthorDate());

    auto newCommitMessage = (message == "" ? commitInfo.getMessage() : std::string{ message });
    auto newCommitDescription = (message == "" ? commitInfo.getDescription() : std::string{ description });

    return createCommitImpl(newCommitMessage, newCommitDescription, commitInfo.getParents(), envp);
}

auto Commits::hasAnyCommits() const -> bool
{
    auto output = repo.executeGitCommand("rev-parse", "--verify", "HEAD");
    if (output.return_code == 0)
    {
        return true;
    }

    if (output.stderr == "fatal: Needed a single revision")
    {
        return false;
    }

    throw std::runtime_error("Failed to check if there are any commits");
}

auto Commits::getHeadCommitHash() const -> std::string
{
    auto branches = repo.Branches();
    return branches.getHashBranchRefersTo("HEAD");
}

auto Commits::getCommitInfo(const std::string_view commitHash) const -> Commit
{
    auto output = repo.executeGitCommand("cat-file", "-p", commitHash);
    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to get commit info");
    }

    return CommitParser::parseCommit_CatFile(output.stdout);
}

auto Commits::createCommitImpl(const std::string_view message, const std::string_view description, const std::vector<std::string>& parents, const std::vector<std::string>& envp) const -> std::string
{
    auto writeTreeOutput = repo.executeGitCommand("write-tree");
    if (writeTreeOutput.return_code != 0)
    {
        throw std::runtime_error("Failed to write tree");
    }

    auto commitArgs = std::vector<std::string>{};
    commitArgs.reserve(1 + 2 * parents.size() + 4); // treeHash + parents*2 + message + description

    commitArgs.push_back(std::move(writeTreeOutput.stdout));

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

    auto commitOutput = (envp.empty() ? repo.executeGitCommand("commit-tree", commitArgs) : repo.executeGitCommand(envp, "commit-tree", commitArgs));

    if (commitOutput.return_code != 0)
    {
        throw std::runtime_error("Failed to create commit");
    }

    const auto& commitHash = commitOutput.stdout;

    const auto branches = repo.Branches();
    branches.changeBranchRef("HEAD", commitHash);


    if (auto updateIndexOutput = repo.executeGitCommand("update-index", "--refresh", "--again", "--quiet"); updateIndexOutput.return_code != 0)
    {
        throw std::runtime_error("Failed to update index");
    }

    return commitHash;
}

} // namespace CppGit

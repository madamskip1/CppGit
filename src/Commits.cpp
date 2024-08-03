#include "Commits.hpp"

#include "GitCommandExecutor/GitCommandOutput.hpp"
#include "Repository.hpp"

namespace CppGit {

Commits::Commits(const Repository& repo)
    : repo(repo)
{
}

void Commits::createCommit(const std::string_view message, const std::string_view description) const
{
    auto writeTreeOutput = repo.executeGitCommand("write-tree");
    if (writeTreeOutput.return_code != 0)
    {
        throw std::runtime_error("Failed to write tree");
    }

    const auto& treeHash = writeTreeOutput.stdout;
    auto parent = hasAnyCommits() ? getHeadCommitHash() : std::string{};

    auto commitOutput = repo.executeGitCommand("commit-tree", treeHash, (parent.empty() ? "" : "-p"), parent, "-m", message, (description.empty() ? "" : "-m"), description);
    if (commitOutput.return_code != 0)
    {
        throw std::runtime_error("Failed to create commit");
    }

    const auto& commitHash = commitOutput.stdout;
    const auto branches = repo.Branches();
    branches.changeBranchRef("HEAD", commitHash);
}

bool Commits::hasAnyCommits() const
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

std::string Commits::getHeadCommitHash() const
{
    auto branches = repo.Branches();
    return branches.getHashBranchRefersTo("HEAD");
}

} // namespace CppGit

#include "Commits.hpp"

#include "Branches.hpp"
#include "GitCommandExecutor/GitCommandOutput.hpp"
#include "Parser/CommitParser.hpp"

namespace CppGit {

Commits::Commits(const Repository& repo)
    : repo(repo)
{
}

auto Commits::createCommit(const std::string_view message, const std::string_view description) const -> std::string
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

    auto x = repo.executeGitCommand("update-index", "--refresh", "--again", "--quiet");

    if (x.return_code != 0)
    {
        throw std::runtime_error("Failed to read tree2");
    }

    return commitHash;
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

} // namespace CppGit

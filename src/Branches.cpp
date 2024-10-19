#include "Branches.hpp"

#include "Branch.hpp"
#include "Commit.hpp"
#include "Parser/BranchesParser.hpp"
#include "Repository.hpp"

namespace CppGit {

Branches::Branches(const Repository& repo)
    : repo(repo)
{
}

auto Branches::getAllBranches() const -> std::vector<Branch>
{
    return getBranchesImpl(true, true);
}

auto Branches::getRemoteBranches() const -> std::vector<Branch>
{
    return getBranchesImpl(false, true);
}

auto Branches::getLocalBranches() const -> std::vector<Branch>
{
    return getBranchesImpl(true, false);
}

auto Branches::getCurrentBranch() const -> Branch
{
    auto currentBranchRef = getCurrentBranchRef();
    auto output = repo.executeGitCommand("for-each-ref", "--format=" + std::string(BranchesParser::BRANCHES_FORMAT), currentBranchRef);
    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to get current branch");
    }

    auto branch = BranchesParser::parseBranch(output.stdout);
    return branch;
}

auto Branches::getCurrentBranchRef() const -> std::string
{
    auto output = repo.executeGitCommand("symbolic-ref", "HEAD");
    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to get current branch ref");
    }

    return output.stdout;
}

auto Branches::changeCurrentBranch(std::string_view branchName) const -> void
{
    auto index = repo.Index();

    if (index.isDirty())
    {
        throw std::runtime_error("Worktree is dirty");
    }

    auto branchNameWithPrefix = addPrefixIfNeeded(branchName, false);
    auto hash = getHashBranchRefersTo(branchNameWithPrefix, false);

    auto output = repo.executeGitCommand("read-tree", "--reset", "-u", hash);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to change current branch");
    }

    output = repo.executeGitCommand("checkout-index", "-a", "-f");

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to change current branch");
    }

    output = repo.executeGitCommand("symbolic-ref", "HEAD", branchNameWithPrefix);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to change current branch");
    }
}

auto Branches::changeCurrentBranch(const Branch& branch) const -> void
{
    return changeCurrentBranch(branch.getRefName());
}

auto Branches::branchExists(std::string_view branchName, bool remote) const -> bool
{
    auto branchNameWithPrefix = addPrefixIfNeeded(branchName, remote);
    auto output = repo.executeGitCommand("show-ref", "--verify", "--quiet", branchNameWithPrefix);
    return output.return_code == 0;
}

auto Branches::branchExists(const Branch& branch) const -> bool
{
    // remote = false/true doesn't matter here
    return branchExists(branch.getRefName(), false);
}

auto Branches::deleteBranch(std::string_view branchName) const -> void
{
    // TODO: delete remote

    auto branchNameWithPrefix = addPrefixIfNeeded(branchName, false);
    auto output = repo.executeGitCommand("update-ref", "-d", branchNameWithPrefix);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to delete branch");
    }
}

auto Branches::deleteBranch(const Branch& branch) const -> void
{
    return deleteBranch(branch.getRefName());
}

auto Branches::getHashBranchRefersTo(std::string_view branchName, bool remote) const -> std::string
{
    auto branchNameWithPrefix = addPrefixIfNeeded(branchName, remote);
    auto output = repo.executeGitCommand("rev-parse", branchNameWithPrefix);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to get hash");
    }

    return output.stdout;
}

auto Branches::getHashBranchRefersTo(const Branch& branch) const -> std::string
{
    // remote = false/true doesn't matter here
    return getHashBranchRefersTo(branch.getRefName(), false);
}

auto Branches::createBranch(std::string_view branchName) const -> void
{
    auto newBranchNameWithPrefix = addPrefixIfNeeded(branchName, false);
    createBranchImpl(newBranchNameWithPrefix, "HEAD");
}

auto Branches::createBranch(const Branch& branch) const -> void
{
    createBranch(branch.getRefName());
}

auto Branches::createBranchFromBranch(std::string_view newBranchName, std::string_view sourceBranch) const -> void
{
    auto newBranchNameWithPrefix = addPrefixIfNeeded(newBranchName, false);
    auto sourceBranchNameWithPrefix = addPrefixIfNeeded(sourceBranch, false);
    createBranchImpl(newBranchNameWithPrefix, sourceBranchNameWithPrefix);
}

auto Branches::createBranchFromBranch(std::string_view newBranchName, const Branch& branch) const -> void
{
    createBranchFromBranch(newBranchName, branch.getRefName());
}

auto Branches::createBranchFromCommit(std::string_view newBranchName, std::string_view commitHash) const -> void
{
    auto newBranchNameWithPrefix = addPrefixIfNeeded(newBranchName, false);
    createBranchImpl(newBranchNameWithPrefix, commitHash);
}

auto Branches::createBranchFromCommit(std::string_view newBranchName, const Commit& commit) const -> void
{
    createBranchFromCommit(newBranchName, commit.getHash());
}

auto Branches::changeBranchRef(std::string_view branchName, std::string_view newHash) const -> void
{
    auto branchNameWithPrefix = addPrefixIfNeeded(branchName, false);
    auto output = repo.executeGitCommand("update-ref", branchNameWithPrefix, newHash);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to change branch ref");
    }
}

auto Branches::changeBranchRef(const Branch& branch, std::string_view newHash) const -> void
{
    changeBranchRef(branch.getRefName(), newHash);
}

auto Branches::getBranchesImpl(bool local, bool remote) const -> std::vector<Branch>
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

auto Branches::createBranchImpl(std::string_view branchName, std::string_view source) const -> void
{
    // TODO: can we create remote branches this way?
    auto output = repo.executeGitCommand("update-ref", branchName, source);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to create branch");
    }
}

auto Branches::addPrefixIfNeeded(std::string_view branchName, bool remote) const -> std::string
{
    if (branchName == "HEAD")
    {
        return std::string{ "HEAD" };
    }

    if (remote)
    {
        if (branchName.find("refs/remotes/") == std::string::npos)
        {
            return REMOTE_BRANCH_PREFIX + std::string{ branchName };
        }
    }
    else
    {
        if (branchName.find("refs/heads/") == std::string::npos)
        {
            return LOCAL_BRANCH_PREFIX + std::string{ branchName };
        }
    }

    return std::string{ branchName };
}

} // namespace CppGit

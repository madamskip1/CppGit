#include "Branches.hpp"

#include "Index.hpp"
#include "_details/Parser/BranchesParser.hpp"

#include <fstream>
#include <utility>

namespace CppGit {

Branches::Branches(const Repository& repo)
    : repo(repo),
      refs(_details::Refs{ repo })
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

auto Branches::getCurrentBranchInfo() const -> Branch
{
    auto currentBranch = getCurrentBranch();
    auto output = repo.executeGitCommand("for-each-ref", "--format=" + std::string{ BranchesParser::BRANCHES_FORMAT }, std::move(currentBranch));

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to get current branch");
    }

    auto branch = BranchesParser::parseBranch(output.stdout);
    return branch;
}

auto Branches::getCurrentBranch() const -> std::string
{
    return refs.getSymbolicRef("HEAD");
}

auto Branches::getCurrentBranchRef() const -> std::string
{
    return refs.getRefHash("HEAD");
}

auto Branches::changeCurrentBranch(std::string_view branchName) const -> void
{
    auto branchNameWithPrefix = refs.getRefWithAddedPrefixIfNeeded(branchName, false);

    changeHEAD(branchNameWithPrefix);
}

auto Branches::changeCurrentBranch(const Branch& branch) const -> void
{
    return changeCurrentBranch(branch.getRefName());
}

auto Branches::detachHead(std::string_view commitHash) const -> void
{
    changeHEAD(commitHash);
}

auto Branches::branchExists(std::string_view branchName, bool remote) const -> bool
{
    auto branchNameWithPrefix = refs.getRefWithAddedPrefixIfNeeded(branchName, remote);

    return refs.refExists(branchNameWithPrefix);
}

auto Branches::branchExists(const Branch& branch) const -> bool
{
    // remote = false/true doesn't matter here
    return branchExists(branch.getRefName(), false);
}

auto Branches::deleteBranch(std::string_view branchName) const -> void
{
    auto branchNameWithPrefix = refs.getRefWithAddedPrefixIfNeeded(branchName, false);

    refs.deleteRef(branchNameWithPrefix);
}

auto Branches::deleteBranch(const Branch& branch) const -> void
{
    return deleteBranch(branch.getRefName());
}

auto Branches::getHashBranchRefersTo(std::string_view branchName, bool remote) const -> std::string
{
    auto branchNameWithPrefix = refs.getRefWithAddedPrefixIfNeeded(branchName, remote);

    return refs.getRefHash(branchNameWithPrefix);
}

auto Branches::getHashBranchRefersTo(const Branch& branch) const -> std::string
{
    // remote = false/true doesn't matter here
    return getHashBranchRefersTo(branch.getRefName(), false);
}

auto Branches::createBranch(std::string_view branchName) const -> void
{
    auto newBranchNameWithPrefix = refs.getRefWithAddedPrefixIfNeeded(branchName, false);
    createBranchImpl(newBranchNameWithPrefix, "HEAD");
}

auto Branches::createBranch(const Branch& branch) const -> void
{
    createBranch(branch.getRefName());
}

auto Branches::createBranchFromBranch(std::string_view newBranchName, std::string_view sourceBranch) const -> void
{
    auto newBranchNameWithPrefix = refs.getRefWithAddedPrefixIfNeeded(newBranchName, false);
    auto sourceBranchNameWithPrefix = refs.getRefWithAddedPrefixIfNeeded(sourceBranch, false);
    createBranchImpl(newBranchNameWithPrefix, sourceBranchNameWithPrefix);
}

auto Branches::createBranchFromBranch(std::string_view newBranchName, const Branch& branch) const -> void
{
    createBranchFromBranch(newBranchName, branch.getRefName());
}

auto Branches::createBranchFromCommit(std::string_view newBranchName, std::string_view commitHash) const -> void
{
    auto newBranchNameWithPrefix = refs.getRefWithAddedPrefixIfNeeded(newBranchName, false);
    createBranchImpl(newBranchNameWithPrefix, commitHash);
}

auto Branches::createBranchFromCommit(std::string_view newBranchName, const Commit& commit) const -> void
{
    createBranchFromCommit(newBranchName, commit.getHash());
}

auto Branches::changeCurrentBranchRef(std::string_view newHash) const -> void
{
    changeBranchRef("HEAD", newHash);
}

auto Branches::changeBranchRef(std::string_view branchName, std::string_view newHash) const -> void
{
    auto branchNameWithPrefix = refs.getRefWithAddedPrefixIfNeeded(branchName, false);
    refs.updateRefHash(branchName, newHash);
}

auto Branches::changeBranchRef(const Branch& branch, std::string_view newHash) const -> void
{
    changeBranchRef(branch.getRefName(), newHash);
}

auto Branches::getBranchesImpl(bool local, bool remote) const -> std::vector<Branch>
{
    const auto* argLocal = local ? refs.LOCAL_BRANCH_PREFIX : "";
    const auto* argRemote = remote ? refs.REMOTE_BRANCH_PREFIX : "";

    auto output = repo.executeGitCommand("for-each-ref", "--format=" + std::string{ BranchesParser::BRANCHES_FORMAT }, argLocal, argRemote);

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
    refs.createRef(branchName, source);
}

auto Branches::changeHEAD(const std::string_view target) const -> void
{
    auto index = repo.Index();

    if (index.isDirty())
    {
        throw std::runtime_error("Worktree is dirty");
    }

    auto HEADFileContent = std::string{};
    auto hash = std::string{};

    if (target.substr(0, 5) == "refs/")
    {
        hash = getHashBranchRefersTo(target);
        refs.updateSymbolicRef("HEAD", target);
    }
    else
    {
        hash = target;
        refs.detachHead(hash);
    }

    auto output = repo.executeGitCommand("read-tree", "--reset", "-u", std::move(hash));

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to change current branch");
    }

    output = repo.executeGitCommand("checkout-index", "-a", "-f");

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to change current branch");
    }
}

} // namespace CppGit

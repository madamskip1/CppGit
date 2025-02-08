#include "Branches.hpp"

#include "Branch.hpp"
#include "Error.hpp"
#include "Index.hpp"
#include "Repository.hpp"
#include "_details/Parser/BranchesParser.hpp"

#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace CppGit {

Branches::Branches(const Repository& repo)
    : repo(repo),
      refs(repo),
      indexWorktree(repo)
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

auto Branches::changeCurrentBranch(const std::string_view branchName) const -> Error
{
    auto branchNameWithPrefix = _details::Refs::getRefWithAddedPrefixIfNeeded(branchName, false);

    return changeHEAD(branchNameWithPrefix);
}

auto Branches::changeCurrentBranch(const Branch& branch) const -> Error
{
    return changeCurrentBranch(branch.getRefName());
}

auto Branches::detachHead(const std::string_view commitHash) const -> Error
{
    return changeHEAD(commitHash);
}

auto Branches::branchExists(const std::string_view branchName, bool remote) const -> bool
{
    auto branchNameWithPrefix = _details::Refs::getRefWithAddedPrefixIfNeeded(branchName, remote);

    return refs.refExists(branchNameWithPrefix);
}

auto Branches::branchExists(const Branch& branch) const -> bool
{
    // remote = false/true doesn't matter here
    return branchExists(branch.getRefName(), false);
}

auto Branches::deleteBranch(const std::string_view branchName) const -> void
{
    auto branchNameWithPrefix = _details::Refs::getRefWithAddedPrefixIfNeeded(branchName, false);

    refs.deleteRef(branchNameWithPrefix);
}

auto Branches::deleteBranch(const Branch& branch) const -> void
{
    deleteBranch(branch.getRefName());
}

auto Branches::getHashBranchRefersTo(const std::string_view branchName, bool remote) const -> std::string
{
    auto branchNameWithPrefix = _details::Refs::getRefWithAddedPrefixIfNeeded(branchName, remote);

    return refs.getRefHash(branchNameWithPrefix);
}

auto Branches::getHashBranchRefersTo(const Branch& branch) const -> std::string
{
    // remote = false/true doesn't matter here
    return getHashBranchRefersTo(branch.getRefName(), false);
}

auto Branches::createBranch(const std::string_view branchName, const std::string_view startRef) const -> void
{
    auto newBranchNameWithPrefix = _details::Refs::getRefWithAddedPrefixIfNeeded(branchName, false);
    refs.createRef(newBranchNameWithPrefix, startRef);
}

auto Branches::createBranch(const Branch& branch, const std::string_view startRef) const -> void
{
    createBranch(branch.getRefName(), startRef);
}

auto Branches::getBranchesImpl(bool local, bool remote) const -> std::vector<Branch>
{
    const auto* argLocal = local ? _details::Refs::LOCAL_BRANCH_PREFIX : "";
    const auto* argRemote = remote ? _details::Refs::REMOTE_BRANCH_PREFIX : "";

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

auto Branches::changeHEAD(const std::string_view target) const -> Error
{

    if (auto index = repo.Index(); index.isDirty())
    {
        return Error::DIRTY_WORKTREE;
    }

    auto hash = std::string{};

    if (target.starts_with("refs/"))
    {
        hash = getHashBranchRefersTo(target);
        refs.updateSymbolicRef("HEAD", target);
    }
    else
    {
        hash = target;
        refs.detachHead(hash);
    }

    indexWorktree.resetIndexToTree(hash);
    indexWorktree.copyForceIndexToWorktree();

    return Error::NO_ERROR;
}

} // namespace CppGit

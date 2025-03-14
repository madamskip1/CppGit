#include "CppGit/Branches.hpp"

#include "CppGit/Branch.hpp"
#include "CppGit/Repository.hpp"
#include "CppGit/_details/GitFilesHelper.hpp"
#include "CppGit/_details/IndexWorktree.hpp"
#include "CppGit/_details/Parser/BranchesParser.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace CppGit {

Branches::Branches(const Repository& repo)
    : repo{ &repo },
      refs{ repo }
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
    auto currentBranchName = getCurrentBranchName();
    const auto output = repo->executeGitCommand("for-each-ref", "--format=" + std::string{ BranchesParser::BRANCHES_FORMAT }, std::move(currentBranchName));

    const auto branch = BranchesParser::parseBranch(output.stdout);
    return branch;
}

auto Branches::getCurrentBranchName() const -> std::string
{
    return refs.getSymbolicRef("HEAD");
}


auto Branches::getCurrentBranchNameOrDetachedHash() const -> std::string
{
    const auto gitFilesHelper = _details::GitFilesHelper{ *repo };
    auto headFileContent = gitFilesHelper.getHeadFile();
    if (headFileContent.starts_with("ref: "))
    {
        constexpr auto REF_PREFIX_SIZE = 5;
        return headFileContent.substr(REF_PREFIX_SIZE);
    }

    return headFileContent;
}

auto Branches::changeCurrentBranch(const std::string_view branchName) const -> void
{
    const auto branchNameWithPrefix = _details::Refs::appendPrefixToRefIfNeeded(branchName, false);
    changeHEAD(branchNameWithPrefix);
}

auto Branches::changeCurrentBranch(const Branch& branch) const -> void
{
    changeCurrentBranch(branch.getRefName());
}

auto Branches::detachHead(const std::string_view commitHash) const -> void
{
    changeHEAD(commitHash);
}

auto Branches::branchExists(const std::string_view branchName, bool remote) const -> bool
{
    const auto branchNameWithPrefix = _details::Refs::appendPrefixToRefIfNeeded(branchName, remote);

    return refs.refExists(branchNameWithPrefix);
}

auto Branches::branchExists(const Branch& branch) const -> bool
{
    // remote = false/true doesn't matter here
    return branchExists(branch.getRefName(), false);
}

auto Branches::deleteBranch(const std::string_view branchName) const -> void
{
    const auto branchNameWithPrefix = _details::Refs::appendPrefixToRefIfNeeded(branchName, false);

    refs.deleteRef(branchNameWithPrefix);
}

auto Branches::deleteBranch(const Branch& branch) const -> void
{
    deleteBranch(branch.getRefName());
}

auto Branches::getHashBranchRefersTo(const std::string_view branchName, bool remote) const -> std::string
{
    const auto branchNameWithPrefix = _details::Refs::appendPrefixToRefIfNeeded(branchName, remote);

    return refs.getRefHash(branchNameWithPrefix);
}

auto Branches::getHashBranchRefersTo(const Branch& branch) const -> std::string
{
    // remote = false/true doesn't matter here
    return getHashBranchRefersTo(branch.getRefName(), false);
}

auto Branches::createBranch(const std::string_view branchName, const std::string_view startRef) const -> void
{
    const auto newBranchNameWithPrefix = _details::Refs::appendPrefixToRefIfNeeded(branchName, false);
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

    const auto output = repo->executeGitCommand("for-each-ref", "--format=" + std::string{ BranchesParser::BRANCHES_FORMAT }, argLocal, argRemote);

    const auto& outputStdoutSV = std::string_view{ output.stdout };
    std::vector<Branch> branches;
    std::size_t start = 0;
    std::size_t delimiterPos = outputStdoutSV.find('\n');

    while (delimiterPos != std::string::npos)
    {
        auto line = outputStdoutSV.substr(start, delimiterPos - start);
        branches.emplace_back(BranchesParser::parseBranch(line));

        start = delimiterPos + 1;
        delimiterPos = outputStdoutSV.find('\n', start);
    }

    if (start < outputStdoutSV.size())
    {
        auto line = outputStdoutSV.substr(start);
        branches.emplace_back(BranchesParser::parseBranch(line));
    }

    return branches;
}

auto Branches::changeHEAD(const std::string_view target) const -> void
{
    const auto hash = [&target, this]() {
        if (target.starts_with("refs/"))
        {
            return getHashBranchRefersTo(target);
        }
        return std::string{ target };
    }();

    if (target.starts_with("refs/"))
    {
        refs.updateSymbolicRef("HEAD", target);
    }
    else
    {
        refs.detachHead(hash);
    }

    const auto indexWorktree = _details::IndexWorktree{ *repo };
    indexWorktree.resetIndexToTree(hash);
    indexWorktree.copyForceIndexToWorktree();
}

} // namespace CppGit

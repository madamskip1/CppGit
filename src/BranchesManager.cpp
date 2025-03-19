#include "CppGit/BranchesManager.hpp"

#include "CppGit/Branch.hpp"
#include "CppGit/Repository.hpp"
#include "CppGit/_details/GitFilesHelper.hpp"
#include "CppGit/_details/IndexWorktreeManager.hpp"
#include "CppGit/_details/Parser/BranchesParser.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace CppGit {

BranchesManager::BranchesManager(const Repository& repository)
    : repository{ &repository },
      referencesManager{ repository }
{
}

auto BranchesManager::getAllBranches() const -> std::vector<Branch>
{
    return getBranchesImpl(true, true);
}

auto BranchesManager::getRemoteBranches() const -> std::vector<Branch>
{
    return getBranchesImpl(false, true);
}

auto BranchesManager::getLocalBranches() const -> std::vector<Branch>
{
    return getBranchesImpl(true, false);
}

auto BranchesManager::getCurrentBranchInfo() const -> Branch
{
    auto currentBranchName = getCurrentBranchName();
    const auto output = repository->executeGitCommand("for-each-ref", "--format=" + std::string{ BranchesParser::BRANCHES_FORMAT }, std::move(currentBranchName));

    const auto branch = BranchesParser::parseBranch(output.stdout);
    return branch;
}

auto BranchesManager::getCurrentBranchName() const -> std::string
{
    return referencesManager.getSymbolicRef("HEAD");
}


auto BranchesManager::getCurrentBranchNameOrDetachedHash() const -> std::string
{
    const auto gitFilesHelper = _details::GitFilesHelper{ *repository };
    auto headFileContent = gitFilesHelper.getHeadFile();
    if (headFileContent.starts_with("ref: "))
    {
        constexpr auto REF_PREFIX_SIZE = 5;
        return headFileContent.substr(REF_PREFIX_SIZE);
    }

    return headFileContent;
}

auto BranchesManager::changeBranch(const std::string_view branchName) const -> void
{
    const auto branchNameWithPrefix = _details::ReferencesManager::appendPrefixToRefIfNeeded(branchName, false);
    changeHEAD(branchNameWithPrefix);
}

auto BranchesManager::changeBranch(const Branch& branch) const -> void
{
    changeBranch(branch.getRefName());
}

auto BranchesManager::detachHead(const std::string_view commitHash) const -> void
{
    changeHEAD(commitHash);
}

auto BranchesManager::branchExists(const std::string_view branchName, bool remote) const -> bool
{
    const auto branchNameWithPrefix = _details::ReferencesManager::appendPrefixToRefIfNeeded(branchName, remote);

    return referencesManager.refExists(branchNameWithPrefix);
}

auto BranchesManager::branchExists(const Branch& branch) const -> bool
{
    // remote = false/true doesn't matter here
    return branchExists(branch.getRefName(), false);
}

auto BranchesManager::deleteBranch(const std::string_view branchName) const -> void
{
    const auto branchNameWithPrefix = _details::ReferencesManager::appendPrefixToRefIfNeeded(branchName, false);

    referencesManager.deleteRef(branchNameWithPrefix);
}

auto BranchesManager::deleteBranch(const Branch& branch) const -> void
{
    deleteBranch(branch.getRefName());
}

auto BranchesManager::getHashBranchRefersTo(const std::string_view branchName, bool remote) const -> std::string
{
    const auto branchNameWithPrefix = _details::ReferencesManager::appendPrefixToRefIfNeeded(branchName, remote);

    return referencesManager.getRefHash(branchNameWithPrefix);
}

auto BranchesManager::getHashBranchRefersTo(const Branch& branch) const -> std::string
{
    // remote = false/true doesn't matter here
    return getHashBranchRefersTo(branch.getRefName(), false);
}

auto BranchesManager::createBranch(const std::string_view branchName, const std::string_view startRef) const -> void
{
    const auto newBranchNameWithPrefix = _details::ReferencesManager::appendPrefixToRefIfNeeded(branchName, false);
    referencesManager.createRef(newBranchNameWithPrefix, startRef);
}

auto BranchesManager::createBranch(const Branch& branch, const std::string_view startRef) const -> void
{
    createBranch(branch.getRefName(), startRef);
}

auto BranchesManager::getBranchesImpl(bool local, bool remote) const -> std::vector<Branch>
{
    const auto* argLocal = local ? _details::ReferencesManager::LOCAL_BRANCH_PREFIX : "";
    const auto* argRemote = remote ? _details::ReferencesManager::REMOTE_BRANCH_PREFIX : "";

    const auto output = repository->executeGitCommand("for-each-ref", "--format=" + std::string{ BranchesParser::BRANCHES_FORMAT }, argLocal, argRemote);

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

auto BranchesManager::changeHEAD(const std::string_view target) const -> void
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
        referencesManager.updateSymbolicRef("HEAD", target);
    }
    else
    {
        referencesManager.detachHead(hash);
    }

    const auto indexWorktreeManager = _details::IndexWorktreeManager{ *repository };
    indexWorktreeManager.resetIndexToTree(hash);
    indexWorktreeManager.copyForceIndexToWorktree();
}

} // namespace CppGit

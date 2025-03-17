#include "CppGit/_details/IndexWorktreeManager.hpp"

#include "CppGit/Repository.hpp"

#include <string_view>

namespace CppGit::_details {

IndexWorktreeManager::IndexWorktreeManager(const Repository& repository)
    : repository{ &repository }
{
}


auto IndexWorktreeManager::copyIndexToWorktree() const -> void
{
    copyIndexToWorktreeImpl(false);
}


auto IndexWorktreeManager::copyForceIndexToWorktree() const -> void
{
    copyIndexToWorktreeImpl(true);
}

auto IndexWorktreeManager::resetIndexToTree(const std::string_view treeHash) const -> void
{
    repository->executeGitCommand("read-tree", "--reset", "-u", treeHash);
}

auto IndexWorktreeManager::copyIndexToWorktreeImpl(const bool force) const -> void
{
    repository->executeGitCommand("checkout-index", "--all", (force ? "--force" : ""));
}

} // namespace CppGit::_details

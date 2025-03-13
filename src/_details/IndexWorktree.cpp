#include "CppGit/_details/IndexWorktree.hpp"

#include "CppGit/Repository.hpp"

#include <string_view>

namespace CppGit::_details {

IndexWorktree::IndexWorktree(const Repository& repo)
    : repo{ &repo }
{
}


auto IndexWorktree::copyIndexToWorktree() const -> void
{
    copyIndexToWorktreeImpl(false);
}


auto IndexWorktree::copyForceIndexToWorktree() const -> void
{
    copyIndexToWorktreeImpl(true);
}

auto IndexWorktree::resetIndexToTree(const std::string_view treeHash) const -> void
{
    repo->executeGitCommand("read-tree", "--reset", "-u", treeHash);
}

auto IndexWorktree::copyIndexToWorktreeImpl(const bool force) const -> void
{
    repo->executeGitCommand("checkout-index", "--all", (force ? "--force" : ""));
}

} // namespace CppGit::_details

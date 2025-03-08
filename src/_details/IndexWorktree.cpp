#include "CppGit/_details/IndexWorktree.hpp"

#include "CppGit/Repository.hpp"

#include <stdexcept>
#include <string_view>

namespace CppGit::_details {

IndexWorktree::IndexWorktree(const Repository& repo)
    : repo(repo)
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
    const auto output = repo.executeGitCommand("read-tree", "--reset", "-u", treeHash);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to reset index to tree");
    }
}

auto IndexWorktree::copyIndexToWorktreeImpl(const bool force) const -> void
{
    const auto output = repo.executeGitCommand("checkout-index", "--all", (force ? "--force" : ""));

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to copy index to worktree");
    }
}

} // namespace CppGit::_details

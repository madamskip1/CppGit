#pragma once

#include "Repository.hpp"

namespace CppGit::_details {

class IndexWorktree
{
public:
    /// @param repo The repository to work with
    explicit IndexWorktree(const Repository& repo);

    /// @brief Copy files from index to worktree
    auto copyIndexToWorktree() const -> void;

    /// @brief Force copy files from index to worktree
    auto copyForceIndexToWorktree() const -> void;

    /// @brief Reset index to worktree state
    /// @param treeHash The tree hash to reset the index to
    auto resetIndexToTree(const std::string_view treeHash) const -> void;

private:
    const Repository& repo;

    auto copyIndexToWorktreeImpl(const bool force) const -> void;
};

} // namespace CppGit::_details

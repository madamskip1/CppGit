#pragma once

#include "Repository.hpp"

namespace CppGit::_details {

class IndexWorktree
{
public:
    explicit IndexWorktree(const Repository& repo);

    auto copyIndexToWorktree() const -> void;
    auto copyForceIndexToWorktree() const -> void;

    auto resetIndexToTree(const std::string_view treeHash) const -> void;

private:
    const Repository& repo;

    auto copyIndexToWorktreeImpl(const bool force) const -> void;
};

} // namespace CppGit::_details

#pragma once

#include "Branch.hpp"
#include "Error.hpp"
#include "Repository.hpp"
#include "_details/IndexWorktree.hpp"
#include "_details/Refs.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace CppGit {

class Branches
{
public:
    explicit Branches(const Repository& repo);
    Branches() = delete;

    auto getAllBranches() const -> std::vector<Branch>;
    auto getRemoteBranches() const -> std::vector<Branch>;
    auto getLocalBranches() const -> std::vector<Branch>;

    auto getCurrentBranchName() const -> std::string;
    auto getCurrentBranchInfo() const -> Branch;

    auto changeCurrentBranch(const std::string_view branchName) const -> Error;
    auto changeCurrentBranch(const Branch& branch) const -> Error;

    auto detachHead(const std::string_view commitHash) const -> Error;

    auto branchExists(const std::string_view branchName, bool remote = false) const -> bool;
    auto branchExists(const Branch& branch) const -> bool;

    auto deleteBranch(const std::string_view branchName) const -> void;
    auto deleteBranch(const Branch& branch) const -> void;

    auto createBranch(const std::string_view branchName, const std::string_view startRef = "HEAD") const -> void;
    auto createBranch(const Branch& branch, const std::string_view startRef = "HEAD") const -> void;

    auto getHashBranchRefersTo(const std::string_view branchName, bool remote = false) const -> std::string;
    auto getHashBranchRefersTo(const Branch& branch) const -> std::string;

private:
    auto getBranchesImpl(bool local, bool remote) const -> std::vector<Branch>;

    auto changeHEAD(const std::string_view target) const -> Error;

    const Repository& repo;
    const _details::Refs refs;
    const _details::IndexWorktree indexWorktree;
};

} // namespace CppGit

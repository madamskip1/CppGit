#pragma once

#include "Branch.hpp"
#include "Error.hpp"
#include "Repository.hpp"
#include "_details/Refs.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace CppGit {

class Branches
{
public:
    /// @param repo The repository to work with
    explicit Branches(const Repository& repo);
    Branches() = delete;

    /// @brief Get all branches (both local and remote) in the repository
    /// @return A vector of all branches in the repository
    [[nodiscard]] auto getAllBranches() const -> std::vector<Branch>;

    /// @brief Get all remote branches in the repository
    /// @return A vector of all remote branches in the repository
    [[nodiscard]] auto getRemoteBranches() const -> std::vector<Branch>;

    /// @brief Get all local branches in the repository
    /// @return A vector of all local branches in the repository
    [[nodiscard]] auto getLocalBranches() const -> std::vector<Branch>;

    /// @brief Get the current branch name
    /// @return The name of the current branch
    [[nodiscard]] auto getCurrentBranchName() const -> std::string;

    /// @brief Get the current branch
    /// @return The current branch
    [[nodiscard]] auto getCurrentBranchInfo() const -> Branch;

    /// @brief Change the current branch
    /// @param branchName The name of the branch to change to
    /// @return An error code
    auto changeCurrentBranch(const std::string_view branchName) const -> Error;

    /// @brief Change the current branch
    /// @param branch The branch to change to
    /// @return An error code
    auto changeCurrentBranch(const Branch& branch) const -> Error;

    /// @brief Detach the HEAD
    ///   Efective it do a checkout to the commit hash
    /// @param commitHash The commit hash to detach the HEAD to
    /// @return An error code
    auto detachHead(const std::string_view commitHash) const -> Error;

    /// @brief Check if a branch exists
    /// @param branchName The name of the branch to check
    /// @param remote If the branch is remote
    /// @return True if the branch exists, false otherwise
    [[nodiscard]] auto branchExists(const std::string_view branchName, bool remote = false) const -> bool;

    /// @brief Check if a branch exists
    /// @param branch The branch to check
    /// @return True if the branch exists, false otherwise
    [[nodiscard]] auto branchExists(const Branch& branch) const -> bool;

    /// @brief Delete a branch
    /// @param branchName The name of the branch to delete
    auto deleteBranch(const std::string_view branchName) const -> void;

    /// @brief Delete a branch
    /// @param branch The branch to delete
    auto deleteBranch(const Branch& branch) const -> void;

    /// @brief Create a branch
    /// @param branchName The name of the branch to create
    /// @param startRef The reference to start the branch from. By default commit is created on top of the HEAD
    auto createBranch(const std::string_view branchName, const std::string_view startRef = "HEAD") const -> void;

    /// @brief Create a branch
    /// @param branch The branch to create
    /// @param startRef The reference to start the branch from. By default commit is created on top of the HEAD
    auto createBranch(const Branch& branch, const std::string_view startRef = "HEAD") const -> void;

    /// @brief Get the commit hash a branch refers to
    /// @param branchName The name of the branch
    /// @param remote If the branch is remote
    /// @return The commit hash the branch refers to
    [[nodiscard]] auto getHashBranchRefersTo(const std::string_view branchName, bool remote = false) const -> std::string;

    /// @brief Get the commit hash a branch refers to
    /// @param branch The branch
    /// @return The commit hash the branch refers to
    [[nodiscard]] auto getHashBranchRefersTo(const Branch& branch) const -> std::string;

private:
    auto getBranchesImpl(bool local, bool remote) const -> std::vector<Branch>;

    auto changeHEAD(const std::string_view target) const -> Error;

    const Repository* repo;

    _details::Refs refs;
};

} // namespace CppGit

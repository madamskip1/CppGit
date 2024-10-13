#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace CppGit {

class Repository; // forward-declaration
class Branch;     // forward-declaration
class Commit;     // forward-declaration

class Branches
{
public:
    static constexpr const char* const LOCAL_BRANCH_PREFIX = "refs/heads/";
    static constexpr const char* const REMOTE_BRANCH_PREFIX = "refs/remotes/";

    explicit Branches(const Repository& repo);
    Branches() = delete;

    auto getAllBranches() const -> std::vector<Branch>;
    auto getRemoteBranches() const -> std::vector<Branch>;
    auto getLocalBranches() const -> std::vector<Branch>;
    auto getCurrentBranch() const -> Branch;
    auto getCurrentBranchRef() const -> std::string;

    auto changeCurrentBranch(std::string_view branchName) const -> void;
    auto changeCurrentBranch(const Branch& branch) const -> void;

    auto branchExists(std::string_view branchName, bool remote = false) const -> bool;
    auto branchExists(const Branch& branch) const -> bool;

    auto deleteBranch(std::string_view branchName) const -> void;
    auto deleteBranch(const Branch& branch) const -> void;

    auto getHashBranchRefersTo(std::string_view branchName, bool remote = false) const -> std::string;
    auto getHashBranchRefersTo(const Branch& branch) const -> std::string;

    auto createBranch(std::string_view branchName) const -> void;
    auto createBranch(const Branch& branch) const -> void;
    auto createBranchFromBranch(std::string_view newBranchName, std::string_view sourceBranch) const -> void;
    auto createBranchFromBranch(std::string_view newBranchName, const Branch& sourceBranch) const -> void;
    auto createBranchFromCommit(std::string_view newBranchName, std::string_view commitHash) const -> void;
    auto createBranchFromCommit(std::string_view newBranchName, const Commit& commit) const -> void;

    auto changeBranchRef(std::string_view branchName, std::string_view newHash) const -> void;
    auto changeBranchRef(const Branch& branch, std::string_view newHash) const -> void;

private:
    auto getBranchesImpl(bool local, bool remote) const -> std::vector<Branch>;
    auto createBranchImpl(std::string_view branchName, std::string_view source) const -> void;
    auto addPrefixIfNeeded(std::string_view branchName, bool remote) const -> std::string;

    const Repository& repo;
};

} // namespace CppGit

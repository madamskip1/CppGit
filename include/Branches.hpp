#pragma once

#include "Branch.hpp"

#include <vector>

namespace CppGit {

class Repository; // forward-declaration

class Branches
{
public:
    static constexpr const char* const LOCAL_BRANCH_PREFIX = "refs/heads/";
    static constexpr const char* const REMOTE_BRANCH_PREFIX = "refs/remotes/";

    explicit Branches(const Repository& repo);
    Branches() = delete;

    std::vector<Branch> getAllBranches() const;
    std::vector<Branch> getRemoteBranches() const;
    std::vector<Branch> getLocalBranches() const;
    std::string getCurrentBranchRef() const;

    bool branchExists(std::string_view branchName, bool remote = false) const;
    bool branchExists(const Branch& branch) const;

    void deleteBranch(std::string_view branchName) const;
    void deleteBranch(const Branch& branch) const;

    std::string getHashBranchRefersTo(std::string_view branchName, bool remote = false) const;
    std::string getHashBranchRefersTo(const Branch& branch) const;

    void createBranch(std::string_view branchName, std::string_view hash = "HEAD") const;
    void createBranchFromBranch(std::string_view newBranchName, const Branch& branch) const;

    void changeBranchRef(std::string_view branchName, std::string_view newHash) const;
    void changeBranchRef(const Branch& branch, std::string_view newHash) const;

private:
    std::vector<Branch> getBranchesImpl(bool local, bool remote) const;
    std::string addPrefixIfNeeded(std::string_view branchName, bool remote) const;

    const Repository& repo;
};

} // namespace CppGit

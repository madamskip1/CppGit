#pragma once

#include "Repository.hpp"
#include "Branch.hpp"

namespace CppGit
{
    class Branches
    {
    public:
        constexpr static const char* const LOCAL_BRANCH_PREFIX = "refs/heads/";
        constexpr static const char* const REMOTE_BRANCH_PREFIX = "refs/remotes/";

        explicit Branches(Repository& repo);

        std::vector<Branch> getAllBranches() const;
        std::vector<Branch> getRemoteBranches() const;
        std::vector<Branch> getLocalBranches() const;

        bool branchExists(std::string_view branchName, bool remote = false) const;
        bool branchExists(const Branch& branch) const;

    private:
        std::vector<Branch> getBranchesImpl(bool local, bool remote) const;
        std::string addPrefixIfNeeded(std::string_view branchName, bool remote) const;

        Repository& repo;
    };
}
#pragma once

#include "Repository.hpp"
#include "Branch.hpp"

namespace CppGit
{
    class Branches
    {
    public:
        explicit Branches(Repository& repo);

        std::vector<Branch> getAllBranches() const;
        std::vector<Branch> getRemoteBranches() const;
        std::vector<Branch> getLocalBranches() const;

    private:
        std::vector<Branch> getBranchesImpl(bool local, bool remote) const;
        Repository& repo;
    };
}
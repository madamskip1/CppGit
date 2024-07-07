#pragma once

#include <vector>
#include <string>
#include "Repository.hpp"

namespace CppGit
{
    class Commit
    {
    public:
        Commit(const Repository& repo, 
            const std::string& hash, 
            const std::vector<std::string>& parents,
            const std::string& authorName,
            const std::string& authorEmail,
            const std::string& authorDate,
            const std::string& committerName,
            const std::string& committerEmail,
            const std::string& committerDate,
            const std::string& message,
            const std::string& description,
            const std::string& tree);

        const std::string& getHash() const;
        const std::vector<std::string>& getParents() const;
        const std::string& getAuthorName() const;
        const std::string& getAuthorEmail() const;
        const std::string& getAuthorDate() const;
        const std::string& getCommitterName() const;
        const std::string& getCommitterEmail() const;
        const std::string& getCommitterDate() const;
        const std::string& getMessage() const;
        const std::string& getDescription() const;
        const std::string& getTree() const;
        
    private:
        const Repository& repo;
        std::string hash;
        std::vector<std::string> parents;
        std::string authorName;
        std::string authorEmail;
        std::string authorDate;
        std::string committerName;
        std::string committerEmail;
        std::string committerDate;
        std::string message;
        std::string description;
        std::string tree;
    };
}
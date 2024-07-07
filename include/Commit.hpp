#pragma once

#include <vector>
#include <string>
#include "Repository.hpp"
#include "Signature.hpp"

namespace CppGit
{
    class Commit
    {
    public:
        Commit(const Repository& repo, 
            const std::string& hash, 
            const std::vector<std::string>& parents,
            const Signature& author,
            const std::string& authorDate,
            const Signature& committer,
            const std::string& committerDate,
            const std::string& message,
            const std::string& description,
            const std::string& treeHash
        );
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
            const std::string& treeHash
        );

        const std::string& getHash() const;
        const std::vector<std::string>& getParents() const;
        const Signature& getAuthor() const;
        const std::string& getAuthorDate() const;
        const Signature& getCommitter() const;
        const std::string& getCommitterDate() const;
        const std::string& getMessage() const;
        const std::string& getDescription() const;
        const std::string& getTreeHash() const;
        
    private:
        const Repository& repo;
        std::string hash;
        std::vector<std::string> parents;
        Signature author;
        std::string authorDate;
        Signature committer;
        std::string committerDate;
        std::string message;
        std::string description;
        std::string treeHash;
    };
}
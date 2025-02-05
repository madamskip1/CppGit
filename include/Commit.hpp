#pragma once

#include "Signature.hpp"

#include <string>
#include <vector>

namespace CppGit {

class Commits;

class Commit
{
    friend Commits;

public:
    Commit(const std::string& hash,
           const std::vector<std::string>& parents,
           const Signature& author,
           const std::string& authorDate,
           const Signature& committer,
           const std::string& committerDate,
           const std::string& message,
           const std::string& description,
           const std::string& treeHash);
    Commit(const std::string& hash,
           const std::vector<std::string>& parents,
           const std::string& authorName,
           const std::string& authorEmail,
           const std::string& authorDate,
           const std::string& committerName,
           const std::string& committerEmail,
           const std::string& committerDate,
           const std::string& message,
           const std::string& description,
           const std::string& treeHash);

    auto getHash() const -> const std::string&;
    auto getParents() const -> const std::vector<std::string>&;
    auto getAuthor() const -> const Signature&;
    auto getAuthorDate() const -> const std::string&;
    auto getCommitter() const -> const Signature&;
    auto getCommitterDate() const -> const std::string&;
    auto getMessage() const -> const std::string&;
    auto getDescription() const -> const std::string&;
    auto getMessageAndDescription() const -> std::string;
    auto getTreeHash() const -> const std::string&;

private:
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

} // namespace CppGit

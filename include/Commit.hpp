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

    [[nodiscard]] auto getHash() const -> const std::string&;
    [[nodiscard]] auto getParents() const -> const std::vector<std::string>&;
    [[nodiscard]] auto getAuthor() const -> const Signature&;
    [[nodiscard]] auto getAuthorDate() const -> const std::string&;
    [[nodiscard]] auto getCommitter() const -> const Signature&;
    [[nodiscard]] auto getCommitterDate() const -> const std::string&;
    [[nodiscard]] auto getMessage() const -> const std::string&;
    [[nodiscard]] auto getDescription() const -> const std::string&;
    [[nodiscard]] auto getMessageAndDescription() const -> std::string;
    [[nodiscard]] auto getTreeHash() const -> const std::string&;

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

#pragma once

#include "Signature.hpp"

#include <string>
#include <vector>

namespace CppGit {

class CommitsManager;

/// @brief Represents a single commit
class Commit
{
    friend CommitsManager;

public:
    Commit(std::string hash,
           std::vector<std::string> parents,
           Signature author,
           std::string authorDate,
           Signature committer,
           std::string committerDate,
           std::string message,
           std::string description,
           std::string treeHash);
    Commit(std::string hash,
           std::vector<std::string> parents,
           std::string authorName,
           std::string authorEmail,
           std::string authorDate,
           std::string committerName,
           std::string committerEmail,
           std::string committerDate,
           std::string message,
           std::string description,
           std::string treeHash);

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
    std::string authorDate;
    std::string committerDate;
    std::string message;
    std::string description;
    std::string treeHash;
    Signature author;
    Signature committer;
    std::vector<std::string> parents;
};

} // namespace CppGit

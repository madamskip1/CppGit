#include "Commit.hpp"
namespace CppGit
{
    Commit::Commit(const Repository &repo, const std::string &hash, const std::vector<std::string> &parents, const std::string &authorName, const std::string &authorEmail, const std::string &authorDate, const std::string &committerName, const std::string &committerEmail, const std::string &committerDate, const std::string &message, const std::string &description, const std::string &treeHash)
        : repo(repo), hash(hash), parents(parents), authorName(authorName), authorEmail(authorEmail), authorDate(authorDate), committerName(committerName), committerEmail(committerEmail), committerDate(committerDate), message(message), description(description), treeHash(treeHash)
    {
    }

    const std::string& Commit::getHash() const
    {
        return hash;
    }

    const std::vector<std::string>& Commit::getParents() const
    {
        return parents;
    }

    const std::string& Commit::getAuthorName() const
    {
        return authorName;
    }

    const std::string& Commit::getAuthorEmail() const
    {
        return authorEmail;
    }

    const std::string& Commit::getAuthorDate() const
    {
        return authorDate;
    }

    const std::string& Commit::getCommitterName() const
    {
        return committerName;
    }

    const std::string& Commit::getCommitterEmail() const
    {
        return committerEmail;
    }

    const std::string& Commit::getCommitterDate() const
    {
        return committerDate;
    }

    const std::string& Commit::getMessage() const
    {
        return message;
    }

    const std::string& Commit::getDescription() const
    {
        return description;
    }

    const std::string& Commit::getTreeHash() const
    {
        return treeHash;
    }
}
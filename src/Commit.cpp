#include "Commit.hpp"

#include "Signature.hpp"

#include <string>
#include <utility>
#include <vector>
namespace CppGit {

Commit::Commit(std::string hash, std::vector<std::string> parents, Signature author, std::string authorDate, Signature committer, std::string committerDate, std::string message, std::string description, std::string treeHash)
    : hash(std::move(hash)),
      authorDate(std::move(authorDate)),
      committerDate(std::move(committerDate)),
      message(std::move(message)),
      description(std::move(description)),
      treeHash(std::move(treeHash)),
      author(std::move(author)),
      committer(std::move(committer)),
      parents(std::move(parents))
{
}

Commit::Commit(std::string hash, std::vector<std::string> parents, std::string authorName, std::string authorEmail, std::string authorDate, std::string committerName, std::string committerEmail, std::string committerDate, std::string message, std::string description, std::string treeHash)
    : Commit(std::move(hash), std::move(parents), Signature(std::move(authorName), std::move(authorEmail)), std::move(authorDate), Signature(std::move(committerName), std::move(committerEmail)), std::move(committerDate), std::move(message), std::move(description), std::move(treeHash))
{
}


auto Commit::getHash() const -> const std::string&
{
    return hash;
}

auto Commit::getParents() const -> const std::vector<std::string>&
{
    return parents;
}

auto Commit::getAuthor() const -> const Signature&
{
    return author;
}

auto Commit::getAuthorDate() const -> const std::string&
{
    return authorDate;
}

auto Commit::getCommitter() const -> const Signature&
{
    return committer;
}

auto Commit::getCommitterDate() const -> const std::string&
{
    return committerDate;
}

auto Commit::getMessage() const -> const std::string&
{
    return message;
}

auto Commit::getDescription() const -> const std::string&
{
    return description;
}

auto Commit::getMessageAndDescription() const -> std::string
{
    if (description.empty())
    {
        return message;
    }

    return message + "\n\n" + description;
}

auto Commit::getTreeHash() const -> const std::string&
{
    return treeHash;
}

} // namespace CppGit

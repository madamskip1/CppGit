#include "Commit.hpp"

#include "GitCommandExecutor/GitCommandExecutorUnix.hpp"
#include "Parser/CommitParser.hpp"

namespace CppGit {

Commit::Commit(const std::string& hash, const std::vector<std::string>& parents, const Signature& author, const std::string& authorDate, const Signature& committer, const std::string& committerDate, const std::string& message, const std::string& description, const std::string& treeHash)
    : hash(hash),
      parents(parents),
      author(author),
      authorDate(authorDate),
      committer(committer),
      committerDate(committerDate),
      message(message),
      description(description),
      treeHash(treeHash)
{
}

Commit::Commit(const std::string& hash, const std::vector<std::string>& parents, const std::string& authorName, const std::string& authorEmail, const std::string& authorDate, const std::string& committerName, const std::string& committerEmail, const std::string& committerDate, const std::string& message, const std::string& description, const std::string& treeHash)
    : Commit(hash, parents, Signature(authorName, authorEmail), authorDate, Signature(committerName, committerEmail), committerDate, message, description, treeHash)
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

auto Commit::getTreeHash() const -> const std::string&
{
    return treeHash;
}

} // namespace CppGit

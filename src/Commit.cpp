#include "Commit.hpp"
#include "GitCommandExecutor.hpp"
#include "CommitParser.hpp"

namespace CppGit
{
    Commit::Commit(const std::string &hash, const std::vector<std::string> &parents, const Signature &author, const std::string &authorDate, const Signature &committer, const std::string &committerDate, const std::string &message, const std::string &description, const std::string &treeHash)
        : hash(hash), parents(parents), author(author), authorDate(authorDate), committer(committer), committerDate(committerDate), message(message), description(description), treeHash(treeHash)
    {
    }

    Commit::Commit(const std::string &hash, const std::vector<std::string> &parents, const std::string &authorName, const std::string &authorEmail, const std::string &authorDate, const std::string &committerName, const std::string &committerEmail, const std::string &committerDate, const std::string &message, const std::string &description, const std::string &treeHash)
        : Commit(hash, parents, Signature(authorName, authorEmail), authorDate, Signature(committerName, committerEmail), committerDate, message, description, treeHash)
    {
    }

    Commit::Commit(const Repository &repo, const std::string_view &hash)
    {
        auto command = std::string{ "show --format=\"" };
        command += CommitParser::COMMIT_LOG_DEFAULT_FORMAT;
        command += "\" --no-patch ";
        command += hash;
        auto output = GitCommandExecutor::execute(command, repo.getPathAsString());

        if (output.return_code != 0)
        {
            throw std::runtime_error("Failed to get commit information");
        }
        
        auto commit = CommitParser::parseCommit(output.output);
        this->hash = commit.hash;
        parents = commit.parents;
        author = commit.author;
        authorDate = commit.authorDate;
        committer = commit.committer;
        committerDate = commit.committerDate;
        message = commit.message;
        description = commit.description;
        treeHash = commit.treeHash;
    }

    const std::string& Commit::getHash() const
    {
        return hash;
    }

    const std::vector<std::string>& Commit::getParents() const
    {
        return parents;
    }

    const Signature& Commit::getAuthor() const
    {
        return author;
    }

    const std::string& Commit::getAuthorDate() const
    {
        return authorDate;
    }

    const Signature& Commit::getCommitter() const
    {
        return committer;
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
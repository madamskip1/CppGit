#include "Parser/CommitParser.hpp"

#include <iostream>

namespace CppGit {

Commit CommitParser::parseCommit(std::string_view commitLog)
{
    return parseCommit(commitLog, COMMIT_LOG_DEFAULT_FORMAT, COMMIT_LOG_DEFAULT_DELIMITER);
}

Commit CommitParser::parseCommit(std::string_view commitLog, std::string_view format, const char delimiter)
{
    const std::vector<std::string_view> commitTokens = split(commitLog, delimiter);
    const std::vector<std::string_view> formatTokens = split(format, delimiter);
    if (commitTokens.size() < formatTokens.size())
    {
        throw std::runtime_error("Invalid format or commit log");
    }
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
    std::string treeHash;

    for (auto i = std::size_t{ 0 }; i < formatTokens.size(); ++i)
    {
        const auto& formatToken = formatTokens[i];
        const auto& commitToken = commitTokens[i];

        if (isHashToken(formatToken))
        {
            hash = commitToken;
        }
        else if (isParentsToken(formatToken))
        {
            auto parents_sv = split(commitToken, ' ');
            if (parents_sv.size() > 1 || (parents_sv.size() == 1 && !parents_sv[0].empty()))
            {
                parents = std::vector<std::string>(parents_sv.begin(), parents_sv.end());
            }
        }
        else if (isAuthorNameToken(formatToken))
        {
            authorName = commitToken;
        }
        else if (isAuthorEmailToken(formatToken))
        {
            authorEmail = commitToken;
        }
        else if (isAuthorDateToken(formatToken))
        {
            authorDate = commitToken;
        }
        else if (isCommitterNameToken(formatToken))
        {
            committerName = commitToken;
        }
        else if (isCommitterEmailToken(formatToken))
        {
            committerEmail = commitToken;
        }
        else if (isCommitterDateToken(formatToken))
        {
            committerDate = commitToken;
        }
        else if (isMessageToken(formatToken))
        {
            message = commitToken;
        }
        else if (isDescriptionToken(formatToken))
        {
            description = commitToken;
        }
        else if (isTreeHashToken(formatToken))
        {
            treeHash = commitToken;
        }
    }

    return Commit(hash, parents, authorName, authorEmail, authorDate, committerName, committerEmail, committerDate, message, description, treeHash);
}

bool CommitParser::isHashToken(std::string_view token)
{
    return token == "%H" || token == "%h";
}

bool CommitParser::isParentsToken(std::string_view token)
{
    return token == "%P" || token == "%p";
}

bool CommitParser::isAuthorNameToken(std::string_view token)
{
    return token == "%an" || token == "%aN";
}

bool CommitParser::isAuthorEmailToken(std::string_view token)
{
    return token == "%ae" || token == "%aE" || token == "%aL";
}

bool CommitParser::isAuthorDateToken(std::string_view token)
{
    return token == "%ad" || token == "%aD" || token == "%ar" || token == "%at" || token == "%ai" || token == "%aI" || token == "%as" || token == "%ah";
}

bool CommitParser::isCommitterNameToken(std::string_view token)
{
    return token == "%cn" || token == "%cN";
}

bool CommitParser::isCommitterEmailToken(std::string_view token)
{
    return token == "%ce" || token == "%cE" || token == "%cL";
}

bool CommitParser::isCommitterDateToken(std::string_view token)
{
    return token == "%cd" || token == "%cD" || token == "%cr" || token == "%ct" || token == "%ci" || token == "%cI" || token == "%cs" || token == "%ch";
}

bool CommitParser::isMessageToken(std::string_view token)
{
    return token == "%s" || token == "%f";
}

bool CommitParser::isDescriptionToken(std::string_view token)
{
    return token == "%b" || token == "%B";
}

bool CommitParser::isTreeHashToken(std::string_view token)
{
    return token == "%T" || token == "%t";
}

} // namespace CppGit

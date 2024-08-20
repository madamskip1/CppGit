#include "Parser/CommitParser.hpp"

#include <iostream>
#include <regex>

namespace CppGit {

Commit CommitParser::parseCommit_CatFile(std::string_view commitLog)
{
    static constexpr auto regexStr = R"(^tree\s+([a-z0-9]{40})(?:\r\n|\r|\n)(parent [\S\s]+?)?author\s+(.+?)\s+<(.+?)>\s+(\d+ [+-]\d{4})(?:\r\n|\r|\n)committer\s+(.+?)\s+<(.+?)>\s+(\d+ [+-]\d{4})(?:\r\n|\r|\n){2}([\S\s]+?)(?:(?:\r\n|\r|\n){2,}|$)([\s\S]+)?$)";
    const auto regex = std::regex{ regexStr };


    auto match = std::cmatch{};
    if (!std::regex_search(commitLog.begin(), commitLog.end(), match, regex))
    {
        throw std::runtime_error("Invalid commit log");
    }

    auto treeHash = match[1].str();
    auto parentStr = match[2].str();
    auto parentSV = split(parentStr, "parent ");
    auto parents = std::vector<std::string>{};
    for (auto parent : parentSV)
    {
        if (!parent.empty())
        {
            parent.remove_suffix(1);
            parents.emplace_back(parent);
        }
    }
    auto authorName = match[3].str();
    auto authorEmail = match[4].str();
    auto authorDate = match[5].str();
    auto committerName = match[6].str();
    auto committerEmail = match[7].str();
    auto committerDate = match[8].str();
    auto message = match[9].str();
    auto description = match[10].str();

    return Commit("", parents, authorName, authorEmail, authorDate, committerName, committerEmail, committerDate, message, description, treeHash);
}


Commit CommitParser::parseCommit_PrettyFormat(std::string_view commitLog)
{
    return parseCommit_PrettyFormat(commitLog, COMMIT_LOG_DEFAULT_FORMAT, COMMIT_LOG_DEFAULT_DELIMITER);
}

Commit CommitParser::parseCommit_PrettyFormat(std::string_view commitLog, std::string_view format, std::string_view delimiter)
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
        auto commitToken = commitTokens[i];

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
            while (!commitToken.empty() && commitToken.back() == '\n')
            {
                commitToken.remove_suffix(1);
            }
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

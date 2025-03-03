#include "_details/Parser/CommitParser.hpp"

#include "Commit.hpp"

#include <cstddef>
#include <regex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace CppGit {

auto CommitParser::parseCommit_CatFile(const std::string_view commitLog) -> Commit
{
    static constexpr auto regexStr = R"(^tree\s+([a-z0-9]{40})(?:\r\n|\r|\n)(parent [\S\s]+?)?author\s+(.+?)\s+<(.+?)>\s+(\d+ [+-]\d{4})(?:\r\n|\r|\n)committer\s+(.+?)\s+<(.+?)>\s+(\d+ [+-]\d{4})(?:\r\n|\r|\n){2}([\S\s]+?)(?:(?:\r\n|\r|\n){2,}|$)([\s\S]+)?$)";
    const auto regex = std::regex{ regexStr };


    auto match = std::cmatch{};
    if (!std::regex_search(commitLog.begin(), commitLog.end(), match, regex))
    {
        throw std::runtime_error("Invalid commit log");
    }

    constexpr auto treeHashIndex = std::size_t{ 1 };
    constexpr auto parentIndex = std::size_t{ 2 };
    constexpr auto authorNameIndex = std::size_t{ 3 };
    constexpr auto authorEmailIndex = std::size_t{ 4 };
    constexpr auto authorDateIndex = std::size_t{ 5 };
    constexpr auto committerNameIndex = std::size_t{ 6 };
    constexpr auto committerEmailIndex = std::size_t{ 7 };
    constexpr auto committerDateIndex = std::size_t{ 8 };
    constexpr auto messageIndex = std::size_t{ 9 };
    constexpr auto descriptionIndex = std::size_t{ 10 };

    auto treeHash = match[treeHashIndex].str();
    const auto parentStr = match[parentIndex].str();
    const auto parentSV = splitToStringViewsVector(parentStr, "parent ");
    auto parents = std::vector<std::string>{};
    for (auto parent : parentSV)
    {
        if (!parent.empty())
        {
            parent.remove_suffix(1);
            parents.emplace_back(parent);
        }
    }
    auto authorName = match[authorNameIndex].str();
    auto authorEmail = match[authorEmailIndex].str();
    auto authorDate = match[authorDateIndex].str();
    auto committerName = match[committerNameIndex].str();
    auto committerEmail = match[committerEmailIndex].str();
    auto committerDate = match[committerDateIndex].str();
    auto message = match[messageIndex].str();
    auto description = match[descriptionIndex].str();

    return { "", std::move(parents), std::move(authorName), std::move(authorEmail), std::move(authorDate), std::move(committerName), std::move(committerEmail), std::move(committerDate), std::move(message), std::move(description), std::move(treeHash) };
}


auto CommitParser::parseCommit_PrettyFormat(const std::string_view commitLog) -> Commit
{
    return parseCommit_PrettyFormat(commitLog, COMMIT_LOG_DEFAULT_FORMAT, COMMIT_LOG_DEFAULT_DELIMITER);
}

auto CommitParser::parseCommit_PrettyFormat(const std::string_view commitLog, const std::string_view format, const std::string_view delimiter) -> Commit
{
    const std::vector<std::string_view> commitTokens = splitToStringViewsVector(commitLog, delimiter);
    const std::vector<std::string_view> formatTokens = splitToStringViewsVector(format, delimiter);
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
            const auto parents_sv = splitToStringViewsVector(commitToken, ' ');
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

    return { std::move(hash), std::move(parents), std::move(authorName), std::move(authorEmail), std::move(authorDate), std::move(committerName), std::move(committerEmail), std::move(committerDate), std::move(message), std::move(description), std::move(treeHash) };
}
auto CommitParser::isHashToken(const std::string_view token) -> bool
{
    return token == "%H" || token == "%h";
}

auto CommitParser::isParentsToken(const std::string_view token) -> bool
{
    return token == "%P" || token == "%p";
}

auto CommitParser::isAuthorNameToken(const std::string_view token) -> bool
{
    return token == "%an" || token == "%aN";
}

auto CommitParser::isAuthorEmailToken(const std::string_view token) -> bool
{
    return token == "%ae" || token == "%aE" || token == "%aL";
}

auto CommitParser::isAuthorDateToken(const std::string_view token) -> bool
{
    return token == "%ad" || token == "%aD" || token == "%ar" || token == "%at" || token == "%ai" || token == "%aI" || token == "%as" || token == "%ah";
}

auto CommitParser::isCommitterNameToken(const std::string_view token) -> bool
{
    return token == "%cn" || token == "%cN";
}

auto CommitParser::isCommitterEmailToken(const std::string_view token) -> bool
{
    return token == "%ce" || token == "%cE" || token == "%cL";
}

auto CommitParser::isCommitterDateToken(const std::string_view token) -> bool
{
    return token == "%cd" || token == "%cD" || token == "%cr" || token == "%ct" || token == "%ci" || token == "%cI" || token == "%cs" || token == "%ch";
}

auto CommitParser::isMessageToken(const std::string_view token) -> bool
{
    return token == "%s" || token == "%f";
}

auto CommitParser::isDescriptionToken(const std::string_view token) -> bool
{
    return token == "%b" || token == "%B";
}

auto CommitParser::isTreeHashToken(const std::string_view token) -> bool
{
    return token == "%T" || token == "%t";
}


} // namespace CppGit

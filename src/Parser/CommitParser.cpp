#include "Parser/CommitParser.hpp"

#include <iostream>
#include <regex>

namespace CppGit {

Commit CommitParser::parseCommit(std::string_view commitLog)
{
    static constexpr auto regexStr = R"(^tree\s+([a-z0-9]{40})(?:\r\n|\r|\n)author\s+(.+?)\s+<(.+?)>\s+(\d+ [+-]\d{4})(?:\r\n|\r|\n)committer\s+(.+?)\s+<(.+?)>\s+(\d+ [+-]\d{4})(?:\r\n|\r|\n){2}([\S\s]+?)(?:(?:\r\n|\r|\n){2,}|$)([\s\S]+)?$)";
    const auto regex = std::regex{ regexStr };


    auto match = std::cmatch{};
    if (!std::regex_search(commitLog.begin(), commitLog.end(), match, regex))
    {
        throw std::runtime_error("Invalid commit log");
    }

    auto treeHash = match[1].str();
    auto authorName = match[2].str();
    auto authorEmail = match[3].str();
    auto authorDate = match[4].str();
    auto committerName = match[5].str();
    auto committerEmail = match[6].str();
    auto committerDate = match[7].str();
    auto message = match[8].str();
    auto description = match[9].str();

    return Commit("", {}, authorName, authorEmail, authorDate, committerName, committerEmail, committerDate, message, description, treeHash);
}

} // namespace CppGit

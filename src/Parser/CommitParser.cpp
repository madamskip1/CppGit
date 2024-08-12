#include "Parser/CommitParser.hpp"

#include <iostream>
#include <regex>

namespace CppGit {

Commit CommitParser::parseCommit(std::string_view commitLog)
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

} // namespace CppGit

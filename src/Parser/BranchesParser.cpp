#include "Parser/BranchesParser.hpp"

#include <iostream>
namespace CppGit {

Branch BranchesParser::parseBranch(std::string_view line)
{
    auto parts = split(line, ';');

    auto refName = std::string{ parts[0] };
    auto upstreamPull = std::string{ parts[1] };
    auto upstreamPush = std::string{ parts[2] };
    auto isLocal = isLocalBranch(parts[0]);
    return Branch(refName, upstreamPull, upstreamPush, isLocal);
}

bool BranchesParser::isLocalBranch(std::string_view refName)
{
    return refName.find("refs/heads/") == 0;
}

} // namespace CppGit

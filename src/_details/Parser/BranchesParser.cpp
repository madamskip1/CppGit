#include "_details/Parser/BranchesParser.hpp"

#include <string>
namespace CppGit {

auto BranchesParser::parseBranch(std::string_view line) -> Branch
{
    auto parts = splitToStringViewsVector(line, ';');

    auto refName = std::string{ parts[0] };
    auto upstreamPull = std::string{ parts[1] };
    auto upstreamPush = std::string{ parts[2] };
    auto isLocal = isLocalBranch(parts[0]);
    return Branch(refName, upstreamPull, upstreamPush, isLocal);
}

auto BranchesParser::isLocalBranch(std::string_view refName) -> bool
{
    return refName.find("refs/heads/") == 0;
}

} // namespace CppGit

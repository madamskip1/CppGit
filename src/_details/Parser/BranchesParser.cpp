#include "_details/Parser/BranchesParser.hpp"

#include "Branch.hpp"

#include <string>
#include <string_view>

namespace CppGit {

auto BranchesParser::parseBranch(const std::string_view line) -> Branch
{
    auto parts = splitToStringViewsVector(line, ';');

    const auto refName = std::string{ parts[0] };
    const auto upstreamPull = std::string{ parts[1] };
    const auto upstreamPush = std::string{ parts[2] };
    const auto isLocal = isLocalBranch(parts[0]);
    return Branch(refName, upstreamPull, upstreamPush, isLocal);
}

auto BranchesParser::isLocalBranch(const std::string_view refName) -> bool
{
    return refName.starts_with("refs/heads/");
}

} // namespace CppGit

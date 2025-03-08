#include "CppGit/_details/Parser/BranchesParser.hpp"

#include "CppGit/Branch.hpp"

#include <string>
#include <string_view>
#include <utility>

namespace CppGit {

auto BranchesParser::parseBranch(const std::string_view line) -> Branch
{
    auto parts = splitToStringViewsVector(line, ';');

    auto refName = std::string{ parts[0] };
    auto upstreamPull = std::string{ parts[1] };
    auto upstreamPush = std::string{ parts[2] };
    const auto isLocal = isLocalBranch(parts[0]);

    return Branch{ std::move(refName), std::move(upstreamPull), std::move(upstreamPush), isLocal };
}

auto BranchesParser::isLocalBranch(const std::string_view refName) -> bool
{
    return refName.starts_with("refs/heads/");
}

} // namespace CppGit

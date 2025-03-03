#include "Branch.hpp"

#include <string>
#include <utility>

namespace CppGit {

Branch::Branch(std::string refName, std::string upstreamPull, std::string upstreamPush, bool isLocalBranch)
    : refName(std::move(refName)),
      upstreamPull(std::move(upstreamPull)),
      upstreamPush(std::move(upstreamPush)),
      isLocal(isLocalBranch)
{
}

auto Branch::getRefName() const -> const std::string&
{
    return refName;
}

auto Branch::getUpstreamPull() const -> const std::string&
{
    return upstreamPull;
}

auto Branch::getUpstreamPush() const -> const std::string&
{
    return upstreamPush;
}

auto Branch::isLocalBranch() const -> bool
{
    return isLocal;
}

} // namespace CppGit

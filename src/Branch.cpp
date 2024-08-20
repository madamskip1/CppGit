#include "Branch.hpp"

namespace CppGit {

Branch::Branch(const std::string& refName, const std::string& upstreamPull, const std::string& upstreamPush, bool isLocalBranch)
    : refName(refName),
      upstreamPull(upstreamPull),
      upstreamPush(upstreamPush),
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

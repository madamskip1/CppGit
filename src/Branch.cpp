#include "Branch.hpp"

namespace CppGit
{
    Branch::Branch(const std::string &refName, const std::string &upstreamPull, const std::string &upstreamPush, bool isLocalBranch)
        : refName(refName), upstreamPull(upstreamPull), upstreamPush(upstreamPush), isLocal(isLocalBranch)
    {
    }

    const std::string &Branch::getRefName() const
    {
        return refName;
    }

    const std::string &Branch::getUpstreamPull() const
    {
        return upstreamPull;
    }

    const std::string &Branch::getUpstreamPush() const
    {
        return upstreamPush;
    }

    bool Branch::isLocalBranch() const
    {
        return isLocal;
    }
} // namespace CppGit
#pragma once

#include <string>

namespace CppGit {

class Branch
{
public:
    explicit Branch(const std::string& refName, const std::string& upstreamPull = "", const std::string& upstreamPush = "", bool isLocalBranch = false);

    auto getRefName() const -> const std::string&;
    auto getUpstreamPull() const -> const std::string&;
    auto getUpstreamPush() const -> const std::string&;
    auto isLocalBranch() const -> bool;

private:
    std::string refName;
    std::string upstreamPull;
    std::string upstreamPush;
    bool isLocal;
};

} // namespace CppGit

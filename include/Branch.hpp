#pragma once

#include <string>
#include <string_view>

namespace CppGit {

class Branch
{
public:
    explicit Branch(const std::string& refName, const std::string& upstreamPull = "", const std::string& upstreamPush = "", bool isLocalBranch = false);

    const std::string& getRefName() const;
    const std::string& getUpstreamPull() const;
    const std::string& getUpstreamPush() const;
    bool isLocalBranch() const;

private:
    std::string refName;
    std::string upstreamPull;
    std::string upstreamPush;
    bool isLocal;
};

} // namespace CppGit

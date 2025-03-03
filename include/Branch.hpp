#pragma once

#include <csignal>
#include <string>

namespace CppGit {

class Branch
{
public:
    /// @param refName The name of the branch
    /// @param upstreamPull The name of remote upstream branch to pull from
    /// @param upstreamPush The name of remote upstream branch to push to
    /// @param isLocalBranch Whether the branch is a local branch or remote
    explicit Branch(std::string refName, std::string upstreamPull = "", std::string upstreamPush = "", bool isLocalBranch = false);

    /// @brief Get the name of the branch
    /// @return The name of the TRAP_BRANCH
    [[nodiscard]] auto getRefName() const -> const std::string&;

    /// @brief Get the name of the remote upstream branch to pull from
    /// @return The name of the remote upstream branch to pull from
    [[nodiscard]] auto getUpstreamPull() const -> const std::string&;

    /// @brief Get the name of the remote upstream branch to push to
    /// @return The name of the remote upstream branch to push to
    [[nodiscard]] auto getUpstreamPush() const -> const std::string&;

    /// @brief Check if the branch is a local branch
    /// @return True if the branch is a local branch, false otherwise
    [[nodiscard]] auto isLocalBranch() const -> bool;

private:
    std::string refName;
    std::string upstreamPull;
    std::string upstreamPush;
    bool isLocal;
};

} // namespace CppGit

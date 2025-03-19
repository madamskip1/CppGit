#pragma once
#include "../Commit.hpp"
#include "../Repository.hpp"

#include <string>
#include <string_view>

namespace CppGit::_details {

/// @brief Provides internal functionality to amend a commit
class CommitAmender
{
public:
    /// @param repo The repository to work with
    explicit CommitAmender(const Repository& repository);
    CommitAmender() = delete;

    /// @brief Amend commit
    ///     The new commit will preserve the message and description of the original commit
    /// @param commit Commit to be amended
    /// @return Amended commit hash
    auto amendCommit(const Commit& commit) const -> std::string;

    /// @brief Amend commit
    /// @param commit Commit to be amended
    /// @param newMessage New commit message
    /// @param newDescription New commit description
    /// @return Amended commit hash
    auto amendCommit(const Commit& commit, const std::string_view newMessage, const std::string_view newDescription = "") const -> std::string;

private:
    const Repository* repository;
};

} // namespace CppGit::_details

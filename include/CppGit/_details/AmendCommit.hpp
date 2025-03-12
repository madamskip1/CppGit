#pragma once
#include "../Commit.hpp"
#include "../Repository.hpp"

#include <string>
#include <string_view>

namespace CppGit::_details {

/// @brief Provides internal functionality to amend a commit
class AmendCommit
{
public:
    /// @param repo The repository to work with
    explicit AmendCommit(const Repository& repo);
    AmendCommit() = delete;

    /// @brief Amend commit
    ///     The new commit will preserve the message and description of the original commit
    /// @param commit Commit to be amended
    /// @return Amended commit hash
    auto amend(const Commit& commit) const -> std::string;

    /// @brief Amend commit
    /// @param commit Commit to be amended
    /// @param newMessage New commit message
    /// @param newDescription New commit description
    /// @return Amended commit hash
    auto amend(const Commit& commit, const std::string_view newMessage, const std::string_view newDescription = "") const -> std::string;

private:
    const Repository* repo;
};

} // namespace CppGit::_details

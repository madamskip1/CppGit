#pragma once

#include "Commit.hpp"
#include "Repository.hpp"
#include "_details/CreateCommit.hpp"

#include <string>
#include <string_view>

namespace CppGit {

class Commits
{
public:
    /// @param repo The repository to work with
    explicit Commits(const Repository& repo);

    /// @brief Create a new commit with the given message and description (optional)
    /// @param message Commit message
    /// @param description Commit description (optional)
    /// @return Hash of the new commit
    auto createCommit(const std::string_view message, const std::string_view description = "") const -> std::string;

    /// @brief Amend the last commit with the given message and description (optional)
    /// @param message Commit message
    /// @param description Commit description (optional)
    /// @return Hash of the amended commit
    auto amendCommit(const std::string_view message = "", const std::string_view description = "") const -> std::string;

    /// @brief Check if repository has any commits
    /// @return True if repository has any commits, false otherwise
    [[nodiscard]] auto hasAnyCommits() const -> bool;

    /// @brief Get the hash of the HEAD commit
    /// @return Hash of the HEAD commit
    [[nodiscard]] auto getHeadCommitHash() const -> std::string;

    /// @brief Get the commit object for the given commit hash with detailed informations
    /// @param commitHash Hash of the commit
    /// @return Commit object
    [[nodiscard]] auto getCommitInfo(const std::string_view commitHash) const -> Commit;

private:
    const Repository* repo;
    _details::CreateCommit _createCommit;
};

} // namespace CppGit

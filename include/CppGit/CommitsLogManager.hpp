#pragma once

#include "Commit.hpp"
#include "Repository.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace CppGit {

/// @brief Provides functionality to get the commits log
class CommitsLogManager
{
public:
    /// @brief Options for including merge commits in log strategy
    enum class LOG_MERGES : uint8_t
    {
        ALL,         ///< Include all commits (default)
        NO_MERGES,   ///< Exclude merge commits
        ONLY_MERGES, ///< Include only merge commits
    };

    /// @brief Order of the commits in the log
    enum class Order : uint8_t
    {
        CHRONOLOGICAL, ///< Chronological order (default)
        REVERSE,       ///< Reverse chronological order
        DATE,          ///< Order by committer date
        AUTHOR_DATE,   ///< Order by author date
        TOPOLOGICAL    ///< Order by topology
    };

    /// @param repo The repository to work with
    explicit CommitsLogManager(const Repository& repository);


    /// @brief Get the commit hashes of the commits in the log
    /// @param ref The reference to start from (default: HEAD)
    /// @return Vector of commit hashes
    [[nodiscard]] auto getCommitsLogHashesOnly(const std::string_view ref = "HEAD") const -> std::vector<std::string>;

    /// @brief Get the commit hashes of the commits in the log from one reference to another
    /// @param fromRef The reference to start from
    /// @param toRef The reference to end at
    /// @return Vector of commit hashes
    [[nodiscard]] auto getCommitsLogHashesOnly(const std::string_view fromRef, const std::string_view toRef) const -> std::vector<std::string>;


    /// @brief Get the commits in the log with detailed informations
    /// @param ref The reference to start from (default: HEAD)
    /// @return Vector of commits
    [[nodiscard]] auto getCommitsLogDetailed(const std::string_view ref = "HEAD") const -> std::vector<Commit>;

    /// @brief Get the commits in the log with detailed informations from one reference to another
    /// @param fromRef The reference to start from
    /// @param toRef The reference to end at
    /// @return Vector of commits
    [[nodiscard]] auto getCommitsLogDetailed(const std::string_view fromRef, const std::string_view toRef) const -> std::vector<Commit>;


    /// @brief Set whether to include all branches in the log
    /// @param allBranches True to include all branches, false otherwise
    /// @return Reference to this object
    auto setAllBranches(const bool allBranches) -> CommitsLogManager&;

    /// @brief Reset whether to include all branches in the log to default (include only the current branch)
    /// @return Reference to this object
    auto resetAllBranches() -> CommitsLogManager&;

    /// @brief Set the maximum number of commits to include in the log
    /// @param maxCount Maximum number of commits
    /// @return Reference to this object
    auto setMaxCount(int maxCount) -> CommitsLogManager&;

    /// @brief Reset the maximum number of commits to include in the log to default (include all commits)
    /// @return Reference to this object
    auto resetMaxCount() -> CommitsLogManager&;

    /// @brief Set the number of commits to skip
    /// @param skip Number of commits to skip
    /// @return Reference to this object
    auto setSkip(int skip) -> CommitsLogManager&;

    /// @brief Reset the number of commits to skip to default (skip no commits)
    /// @return Reference to this object
    auto resetSkip() -> CommitsLogManager&;

    /// @brief Set whether to include merges in the log
    /// @param logMerges Log merges strategy
    /// @return Reference to this object
    auto setLogMerges(LOG_MERGES logMerges) -> CommitsLogManager&;

    /// @brief Reset whether to include merges in the log to default (include all commits)
    /// @return Reference to this object
    auto resetLogMerges() -> CommitsLogManager&;

    /// @brief Set the order of the commits in the log
    /// @param order Order of the commits
    /// @return Reference to this object
    auto setOrder(Order order) -> CommitsLogManager&;

    /// @brief Reset the order of the commits in the log to default (chronological)
    /// @return Reference to this object
    auto resetOrder() -> CommitsLogManager&;

    /// @brief Set the author pattern to filter the commits
    /// @param authorPattern Author pattern
    /// @return Reference to this object
    auto setAuthorPattern(const std::string_view authorPattern) -> CommitsLogManager&;

    /// @brief Reset the author pattern to default (no filter)
    /// @return Reference to this object
    auto resetAuthorPattern() -> CommitsLogManager&;

    /// @brief Set the committer pattern to filter the commits
    /// @param committerPattern Committer pattern
    /// @return Reference to this object
    auto setCommitterPattern(const std::string_view committerPattern) -> CommitsLogManager&;

    /// @brief Reset the committer pattern to default (no filter)
    /// @return Reference to this object
    auto resetCommitterPattern() -> CommitsLogManager&;

    /// @brief Set the message pattern to filter the commits
    /// @param messagePattern Message pattern
    /// @return Reference to this object
    auto setMessagePattern(const std::string_view messagePattern) -> CommitsLogManager&;

    /// @brief Reset the message pattern to default (no filter)
    /// @return Reference to this object
    auto resetMessagePattern() -> CommitsLogManager&;

private:
    const Repository* repository;

    bool allBranches_{ false };
    int maxCount_{ -1 };
    int skip_{ -1 };
    LOG_MERGES logMerges_{ LOG_MERGES::ALL };
    Order order_{ Order::CHRONOLOGICAL };
    std::string authorPattern_;
    std::string committerPattern_;
    std::string messagePattern_;

    auto prepareCommandsArgument(const std::string_view fromRef, const std::string_view toRef) const -> std::vector<std::string>;

    auto getCommitsLogHashesOnlyImpl(const std::string_view fromRef, const std::string_view toRef) const -> std::vector<std::string>;
    auto getCommitsLogDetailedImpl(const std::string_view fromRef, const std::string_view toRef) const -> std::vector<Commit>;
};

} // namespace CppGit

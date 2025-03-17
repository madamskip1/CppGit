#pragma once
#include "../Repository.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace CppGit::_details {

/// @brief Provides internal functionality to create a commit
class CommitCreator
{
public:
    /// @param repo The repository to work with
    explicit CommitCreator(const Repository& repository);
    CommitCreator() = delete;

    /// @brief Create commit
    /// @param message Commit message
    /// @param description Commit description
    /// @param parents Parent commit hashes
    /// @param envp Environment variables
    /// @return Commit hash
    auto createCommit(const std::string_view message, const std::string_view description, const std::vector<std::string>& parents, const std::vector<std::string>& envp) const -> std::string;

    /// @brief Create commit
    /// @param message Commit message
    /// @param parents Parent commit hashes
    /// @param envp Environment variables
    /// @return Commit hash
    auto createCommit(const std::string_view message, const std::vector<std::string>& parents, const std::vector<std::string>& envp) const -> std::string;


private:
    const Repository* repository;

    auto writeTree() const -> std::string;

    auto commitTree(std::string&& treeHash, const std::string_view message, const std::string_view description, const std::vector<std::string>& parents, const std::vector<std::string>& envp) const -> std::string;
    auto commitTreeImpl(std::vector<std::string> commitArgs, const std::vector<std::string>& envp = {}) const -> std::string;
};

} // namespace CppGit::_details

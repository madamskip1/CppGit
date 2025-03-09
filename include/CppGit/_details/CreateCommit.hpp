#pragma once
#include "../Repository.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace CppGit::_details {

class CreateCommit
{
public:
    /// @param repo The repository to work with
    explicit CreateCommit(const Repository& repo);
    CreateCommit() = delete;

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
    const Repository* repo;

    auto writeTree() const -> std::string;

    auto commitTree(std::string&& treeHash, const std::string_view message, const std::string_view description, const std::vector<std::string>& parents, const std::vector<std::string>& envp) const -> std::string;
    auto commitTreeImpl(std::vector<std::string> commitArgs, const std::vector<std::string>& envp = {}) const -> std::string;
};

} // namespace CppGit::_details

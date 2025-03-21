#pragma once

#include "../Repository.hpp"

namespace CppGit::_details {

/// @brief Provides internal functionality to work with references
class ReferencesManager
{
public:
    static constexpr const char* const LOCAL_BRANCH_PREFIX = "refs/heads/";    ///< Prefix for local branches refs/heads/<branch_name>
    static constexpr const char* const REMOTE_BRANCH_PREFIX = "refs/remotes/"; ///< Prefix for remote branches refs/remotes/<remote_name>/<branch_name>

    /// @param repo The repository to work with
    explicit ReferencesManager(const Repository& repository);

    /// @brief Get the hash of a reference
    /// @param refName The name of the reference
    /// @return The hash of the reference
    [[nodiscard]] auto getRefHash(const std::string_view refName) const -> std::string;

    /// @brief Get the symbolic reference that reference points to
    /// @param refName Reference name
    /// @return The symbolic reference
    [[nodiscard]] auto getSymbolicRef(const std::string_view refName) const -> std::string;

    /// @brief Check if a reference exists
    /// @param refName The name of the reference
    /// @return True if the reference exists, false otherwise
    [[nodiscard]] auto refExists(const std::string_view refName) const -> bool;

    /// @brief Update the hash that refName points to
    /// @param refName The name of the reference
    /// @param newHash The new hash to point to
    auto updateRefHash(const std::string_view refName, const std::string_view newHash) const -> void;

    /// @brief Update the symbolic reference that reference points to
    /// @param refName Reference name
    /// @param newRef The new symbolic reference
    auto updateSymbolicRef(const std::string_view refName, const std::string_view newRef) const -> void;

    /// @brief Delete a reference
    /// @param refName Reference name
    auto deleteRef(const std::string_view refName) const -> void;

    /// @brief Create a reference
    /// @param refName Reference name
    /// @param hash The hash to point to
    auto createRef(const std::string_view refName, const std::string_view hash) const -> void;

    /// @brief Detach HEAD
    /// @param commitHash The hash of the commit to detach HEAD to
    auto detachHead(const std::string_view commitHash) const -> void;

    /// @brief Append the correct prefix to a reference name if needed
    /// @param refName Reference name
    /// @param remote True if the reference is a remote reference
    /// @return The reference name with the correct prefix
    [[nodiscard]] static auto appendPrefixToRefIfNeeded(const std::string_view refName, bool remote) -> std::string;

private:
    const Repository* repository;
};

} // namespace CppGit::_details

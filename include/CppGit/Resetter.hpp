#pragma once

#include "Repository.hpp"
#include "_details/GitFilesHelper.hpp"
#include "_details/ReferencesManager.hpp"

namespace CppGit {

/// @brief Provides functionality to reset the repository to a specific commit
class Resetter
{

public:
    /// @param repo The repository to work with
    explicit Resetter(const Repository& repository);

    /// @brief Reset the repository to the given commit hash and keep the changes
    /// @param commitHash Hash of the commit to reset to
    auto resetSoft(const std::string_view commitHash) const -> void;

    /// @brief Reset the repository to the given commit hash and discard the changes
    /// @param commitHash Hash of the commit to reset to
    auto resetHard(const std::string_view commitHash) const -> void;

private:
    const Repository* repository;

    _details::GitFilesHelper gitFilesHelper;
    _details::ReferencesManager referencesManager;
};

} // namespace CppGit

#pragma once

#include "Repository.hpp"
#include "_details/GitFilesHelper.hpp"
#include "_details/Refs.hpp"

namespace CppGit {

class Reset
{

public:
    /// @param repo The repository to work with
    explicit Reset(const Repository& repo);

    /// @brief Reset the repository to the given commit hash and keep the changes
    /// @param commitHash Hash of the commit to reset to
    auto resetSoft(const std::string_view commitHash) const -> void;

    /// @brief Reset the repository to the given commit hash and discard the changes
    /// @param commitHash Hash of the commit to reset to
    auto resetHard(const std::string_view commitHash) const -> void;

private:
    const Repository* repo;

    _details::GitFilesHelper gitFilesHelper;
    _details::Refs refs;
};

} // namespace CppGit

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
    explicit Commits(const Repository& repo);

    auto createCommit(const std::string_view message, const std::string_view description = "") const -> std::string;
    auto amendCommit(const std::string_view message = "", const std::string_view description = "") const -> std::string;

    auto hasAnyCommits() const -> bool;

    auto getHeadCommitHash() const -> std::string;
    auto getCommitInfo(const std::string_view commitHash) const -> Commit;

private:
    const Repository& repo;
    const _details::CreateCommit _createCommit;
};

} // namespace CppGit
